#include <cutestl/unique_ptr.h>

#include <atomic>
#include <cstddef>
#include <cstdio>
#include <type_traits>
#include <utility>

namespace cutestl {

// 智能指针控制块
struct SpCounter {
    // ctor
    // NOTE: 使用 new 创造的 SpCounter 的 refcnt 为 1
    explicit SpCounter() : refcnt_(1) {}

    // 删除 mvctor
    // NOTE: 同时也会删除 cpctor cpassign mvassign
    SpCounter(SpCounter&&) = delete;

    // dtor
    virtual ~SpCounter() = default;

    // 增加引用计数
    // NOTE: 原子操作
    void IncRef() {
        // HACK: 没见过第二个参数
        refcnt_.fetch_add(1, std::memory_order_relaxed);  // refcnt_++ 返回旧值
    }

    // 减少引用计数
    // NOTE: 原子操作
    void DecRef() {
        // 当引用计数为 0 时再释放控制块指针
        if (refcnt_.fetch_sub(1, std::memory_order_relaxed) == 1) {  // refcnt_-- 返回旧值
            delete this;  // HACK: 可以释放[外部] new SpCounter 动态内存
        }
    }

    // 返回当前引用计数
    int CntRef() const noexcept { return refcnt_.load(std::memory_order_relaxed); }

    std::atomic<int> refcnt_;  // 引用计数(HACK: 原子变量)
};

template <typename T, typename Deleter>
struct SpCounterImpl : SpCounter {
    T* ptr_;
    Deleter deleter_;
    explicit SpCounterImpl(T* ptr) noexcept : ptr_(ptr) {}
    // HACK: std::move(deleter)
    explicit SpCounterImpl(T* ptr, Deleter deleter) noexcept : ptr_(ptr), deleter_(deleter) {}
    ~SpCounterImpl() noexcept override { deleter_(ptr_); }
};

template <typename T>
class SharedPtr {
public:
    // 支持  auto p = SharedPtr<T>(); 或 SharedPtr<T> p = nullptr;
    // 1. SharedPtr() : owner_(nullptr) {}
    // 2. SharedPtr(std::nullptr_t) : owner_(nullptr) {}
    // 合二为一
    SharedPtr(std::nullptr_t = nullptr) noexcept : owner_(nullptr) {}

    template <typename Y, std::enable_if_t<std::is_convertible_v<Y*, T*>, int> = 0>
    explicit SharedPtr(Y* ptr) : ptr_(ptr), owner_(new SpCounterImpl) {}

    // explicit SharedPtr(T* ptr) : owner_(new SpCounter<T>(ptr)) {
    // printf("%s: refcnt: %d\n", __PRETTY_FUNCTION__, owner_->refcnt_.load());
    // }

    SharedPtr(SharedPtr const& other) {
        owner_ = other.owner_;
        ++owner_->refcnt_;
        // printf("%s: refcnt: %d\n", __PRETTY_FUNCTION__, owner_->refcnt_.load());
    }

    ~SharedPtr() {
        --owner_->refcnt_;
        // printf("%s: refcnt: %d\n", __PRETTY_FUNCTION__, owner_->refcnt_.load());
        owner_->DecRef();  // NOTE: 将原来的 delete owner_ 放进 DecRefCnt() 中
    }
    T* Get() const { return owner_->data_; }

private:
    SpCounter* owner_;
    T* ptr_;

    // HACK:
    explicit SharedPtr(T* ptr, SpCounter* owner) noexcept : ptr_(ptr), owner_(owner) {}
};

// TODO: 一般实现 make_shared 会同时分配 控制块 和 T, 考虑内存对齐
// 这里偷懒了

// TODO: 还是需要经常复习万能引用与完美转发
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    return SharedPtr<T>{new T{std::forward<Args>(args)...}};
}

}  // namespace cutestl

struct Student {
    char const* name_;
    int age_;
    explicit Student(char const* name = "Student", int age = 24) : name_(name), age_(age) {}
    ~Student() { puts(__PRETTY_FUNCTION__); }
};

struct StudentDerived : Student {
    ~StudentDerived() { puts(__PRETTY_FUNCTION__); }
};

int main() {
    using namespace cutestl;
    auto p0 = MakeShared<Student>();
    auto p1 = p0;
    auto p2 = MakeShared<int>();
    p2 = nullptr;
    // auto p2 = p0;
    return 0;
}
