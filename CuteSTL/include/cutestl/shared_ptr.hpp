#include <atomic>
#include <compare>
#include <concepts>
#include <cstddef>
#include <functional>
#include <iostream>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

// NOTE: copy-and-swap: 拷贝构造函数 + 交换
// 1. 对于赋值运算符具有强异常安全保证
// 2. 不需要检查自赋值

// C++20 Concept: 检查 From* 是否可以转换为 To*
template <typename From, typename To>
concept PointerConvertible = std::is_convertible_v<From*, To*>;

// 内部控制块结构
struct ControlBlock {
    std::atomic<size_t> ref_cnt_{1};      // 引用计数 NOTE: 初始值 1
    std::function<void(void*)> deleter_;  // 类型擦除删除器

    // 构造函数，接收一个指针和对应的删除器
    template <typename U, typename Deleter>
    ControlBlock(U*, Deleter d) {  // NOTE: 指针类型 U 后面用于转换
        // 使用 lambda 捕获具体的删除器，并将其包装进 std::function
        // 无论原始删除器是函数指针还是带状态的函数对象，都可以被正确存储和调用
        deleter_ = [deleter = std::move(d)](void* ptr_to_delete) {
            deleter(static_cast<U*>(ptr_to_delete));  // NOTE: 需要强转回原来的类型才能正确析构!
        };
    }
};

// 默认删除器
template <typename T>
struct DefaultDeleter {
    void operator()(T* ptr) const { delete ptr; }
};

template <typename T>
class SharedPtr {
    template <typename U>
    friend class SharedPtr;  // 允许所有 SharedPtr<U> 访问私有成员
public:
    // 默认构造函数，创建一个空的 SharedPtr
    SharedPtr() noexcept = default;

    // 空指针构造函数
    SharedPtr(std::nullptr_t) noexcept {}

    // 从原始指针构造 SharedPtr (默认删除器)
    // DefaultDeleter 模板参数使用原始的静态类型 U
    template <PointerConvertible<T> U>  // NOTE: 写法含义: U 可以隐式转换为 T
    explicit SharedPtr(U* ptr)
        : p_(ptr), cb_(ptr ? new ControlBlock{ptr, DefaultDeleter<U>{}} : nullptr) {}

    // 从原始指针和自定义删除器构造
    template <PointerConvertible<T> U, typename Deleter>
    SharedPtr(U* ptr, Deleter deleter)
        : p_(ptr), cb_(ptr ? new ControlBlock{ptr, std::move(deleter)} : nullptr) {}

    // 拷贝构造函数
    SharedPtr(const SharedPtr& other) noexcept : p_(other.p_), cb_(other.cb_) {
        if (cb_) {
            cb_->ref_cnt_.fetch_add(1, std::memory_order_relaxed);
        }
    }

    // 拷贝构造函数 (模板化)
    template <PointerConvertible<T> U>
    SharedPtr(const SharedPtr<U>& other) noexcept : p_(other.p_), cb_(other.cb_) {
        if (cb_) {
            cb_->ref_cnt_.fetch_add(1, std::memory_order_relaxed);
        }
    }

    // 移动构造函数
    SharedPtr(SharedPtr&& other) noexcept {
        p_ = std::exchange(other.p_, nullptr);
        cb_ = std::exchange(other.cb_, nullptr);
    }

    // 移动构造函数 (模板化)
    template <PointerConvertible<T> U>
    SharedPtr(SharedPtr<U>&& other) noexcept {
        p_ = std::exchange(other.p_, nullptr);
        cb_ = std::exchange(other.cb_, nullptr);
    }

    // 析构函数
    ~SharedPtr() { Release(); }

    // 拷贝赋值运算符 (copy-and-swap)
    SharedPtr& operator=(const SharedPtr& other) noexcept {
        SharedPtr{other}.Swap(*this);
        return *this;
    }

    // 拷贝赋值运算符 (模板化)
    template <PointerConvertible<T> U>
    SharedPtr& operator=(const SharedPtr<U>& other) noexcept {
        SharedPtr{other}.Swap(*this);
        return *this;
    }

    // 移动赋值运算符
    SharedPtr& operator=(SharedPtr&& other) noexcept {
        SharedPtr{std::move(other)}.Swap(*this);
        return *this;
    }

    // 移动赋值运算符 (模板化)
    template <PointerConvertible<T> U>
    SharedPtr& operator=(SharedPtr<U>&& other) noexcept {
        SharedPtr{std::move(other)}.Swap(*this);
        return *this;
    }

    // 重置智能指针
    void Reset() noexcept { Release(); }

    template <typename U>
    void Reset(U* ptr) {
        SharedPtr{ptr}.Swap(*this);
    }

    template <typename U, typename Deleter>
    void Reset(U* ptr, Deleter deleter) {
        SharedPtr{ptr, std::move(deleter)}.Swap(*this);
    }

    // NOTE: 交换两个智能指针的内容
    void Swap(SharedPtr& other) noexcept {
        std::swap(p_, other.p_);
        std::swap(cb_, other.cb_);
    }

    // 获取原始指针
    T* Get() const noexcept { return p_; }

    // 解引用操作
    T& operator*() const noexcept { return *p_; }

    // 成员访问操作
    T* operator->() const noexcept { return p_; }

    // 获取引用计数
    size_t UseCount() const noexcept {
        return cb_ ? cb_->ref_cnt_.load(std::memory_order_acquire) : 0;
    }

    // 检查是否拥有对象
    explicit operator bool() const noexcept { return p_ != nullptr; }

    // C++20 比较运算符
    // 统一比较，与另一个 SharedPtr
    auto operator<=>(const SharedPtr& other) const noexcept { return p_ <=> other.p_; }

    // 与 nullptr 的比较
    auto operator<=>(std::nullptr_t) const noexcept { return p_ <=> nullptr; }

private:
    // NOTE: 经典 Release !!
    void Release() {
        if (!cb_) {
            return;
        }
        // fetch_sub 和 acquire-release 语义
        if (cb_->ref_cnt_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            cb_->deleter_(p_);
            delete cb_;
        }
        p_ = nullptr;
        cb_ = nullptr;
    }

private:
    T* p_{nullptr};
    ControlBlock* cb_{nullptr};
};
