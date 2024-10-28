#if __cplusplus >= 202002L
#include <concepts>
#endif
#include <cstdio>
#include <iostream>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

using std::cout;
using std::endl;

// 创建一个可调用对象的删除器（避免使用 std::function）
template <typename T>
struct DefaultDeleter {
    void operator()(T* p) const {
        // std::cout << "DefaultDeleter: delete" << std::endl;
        delete p;  // 默认使用 delete 释放内存
    }
};

// DefaultDeleter 对数组类型的偏特化
// NOTE: 这里 T[] 其实有所特指
// 数组一般会退化为指针
template <typename T>
struct DefaultDeleter<T[]> {
    void operator()(T* p) const {
        // std::cout << "DefaultDeleter: delete []" << std::endl;
        delete[] p;
    }
};

template <typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr {
private:
    T* ptr_ = nullptr;  // 原始指针
    Deleter deleter_;   // 删除器

    // NOTE: 友元类，为了能在 std::exchange 中访问 other.ptr_
    template <typename U, typename UDeleter>
    friend class UniquePtr;

public:
    // NOTE: 为了能支持
    // 1. UniquePtr = nullptr;
    // 2. 空构造()
    // 也可以直接使用 UniquePtr.Reset() 方法
    UniquePtr(std::nullptr_t dummy = nullptr) noexcept : ptr_(nullptr) {}

    // NOTE: 闪回：避免由普通指针而不是指向动态内存的指针隐式构造
    explicit UniquePtr(T* ptr) noexcept : ptr_(ptr) {}

    ~UniquePtr() noexcept {
        if (ptr_) {
            deleter_(ptr_);
        }
    }

public:
    // 删除 cpctor
    UniquePtr(UniquePtr const& other) = delete;

    // mvctor
    UniquePtr(UniquePtr&& other) : ptr_(other.ptr_) { other.ptr_ = nullptr; }

public:
    // 删除 cpassign
    UniquePtr& operator=(UniquePtr const& other) = delete;

    // mvassign
    UniquePtr& operator=(UniquePtr&& other) {
        // 判断是否自赋值
        if (this != &other) [[likely]] {
            if (ptr_) {
                deleter_(ptr_);
            }
            std::exchange(other.ptr_, nullptr);
        }
        return *this;
    }

    // -> 运算符
    // NOTE: 返回指针
    // NOTE: -> 自己递归下去直到找到原始指针
    // 编译器能解引用
    // WARN: 这里 const 是有说法的
    // 为了能被 auto const& 使用
    T* operator->() const { return ptr_; }

    // 解引用 * 运算符
    // NOTE: 返回引用
    T& operator*() const { return *ptr_; }

    // TODO: 不知道这是啥，而且还报错
    // std::add_lvalue_reference_t<T> operator*() const noexcept { return *ptr_; }

    explicit operator bool() const noexcept { return ptr_ != nullptr; }

// bool operator==(UniquePtr const& other)const noexcept{
//     return ptr_==
// }
#if __cplusplus >= 202002L
    bool operator<=>(UniquePtr const& other) const noexcept { return ptr_ <=> other.ptr_; }
#else
    bool operator==(UniquePtr const& other) const noexcept { return ptr_ == other.ptr_; }

    bool operator!=(UniquePtr const& other) const noexcept { return ptr_ != other.ptr_; }

    bool operator<(UniquePtr const& other) const noexcept { return ptr_ < other.ptr_; }

    bool operator<=(UniquePtr const& other) const noexcept { return ptr_ <= other.ptr_; }

    bool operator>(UniquePtr const& other) const noexcept { return ptr_ > other.ptr_; }

    bool operator>=(UniquePtr const& other) const noexcept { return ptr_ >= other.ptr_; }

#endif

    // 类型转换（支持多态）
    // NOTE: new 原始指针支持 U* -> T*
    // 这里希望支持 UniquePtr<U> -> UniquePtr<T>
    // 法一：类型转换运算符
    // operator U() const {}
    // 法二：采用构造函数的隐式转换 [x]
#if __cplusplus >= 202002L
    template <typename U, typename UDeleter>
        requires(std::convertible_to<U*, T*>)  // C++20 ->
#else
    template <typename U, typename UDeleter,
              std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>  // C++11 ->
#endif
    UniquePtr(UniquePtr<U, UDeleter>&& other) noexcept {
        ptr_ = std::exchange(other.ptr_, nullptr);
    }

public:
    T* Get() const { return ptr_; }

    T* Release() { return std::exchange(ptr_, nullptr); }

    void Reset(T* other_ptr = nullptr) {
        if (ptr_) {
            deleter_(ptr_);
        }
        ptr_ = other_ptr;
    }

    void Swap(UniquePtr& other) noexcept { std::swap(ptr_, other.ptr_); }
    Deleter GetDeleter() const noexcept { return deleter_; }
};

// UniquePtr 针对 T[] 的偏特化
// NOTE: 偏特化模板参数不能有默认值
// 由于Deleter = DefaultDeleter<T>
// 则 UniquePtr<T[], Deleter> 中 T[] 传递给 DefaultDeleter
// NOTE: 这里使用继承是偷懒么？这个偏特化继承主模板
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> : public UniquePtr<T, Deleter> {
    using UniquePtr<T, Deleter>::UniquePtr;  // HACK: 继承构造函数

    std::add_lvalue_reference_t<T> operator[](std::size_t i) {
        return this->Get()[i];  // HACK: 此处用 this
    }
};

// ---------------------------------

// NOTE: 为什么使用万能引用与完美转发？
// 1. 如果 Cat 类中存在引用类型的成员变量，则以引用形式捕获
// 2. 如果 Dog 类中没有引用类型的成员变量，则以普通形式捕获
template <typename T, typename... Args>
UniquePtr<T> MakeUnique(Args&&... args) {
    // NOTE: 闪回：完美转发
    // NOTE: T{args...} 花括号初始化没有构造函数的类对象[C++11]
    return UniquePtr<T>{new T{std::forward<Args>(args)...}};
    // NOTE: T(args...) 圆括号初始化没有构造函数的类对象[only C++20]
    // return UniquePtr<T>{new T(std::forward<Args>(args)...)};
}

struct Animal {
    virtual void Speak() = 0;
    virtual ~Animal() {}
};

// HACK: 继承自虚基类不能没有构造函数
struct Cat : public Animal {
    Cat(int& age) : age_(age) {}
    int& age_;
    void Speak() override { cout << "Cat Speak " << age_ << endl; }
};

struct Dog : public Animal {
    Dog(int age) : age_(age) {}
    int age_;
    void Speak() override { cout << "Dog Speak " << age_ << endl; }
};

int main() {
    std::vector<UniquePtr<Animal>> zoo;
    // std::vector<Animal*> zoo{new Cat(), new Dog()};
    int age = 3;
    zoo.push_back(MakeUnique<Cat>(age));
    zoo.push_back(MakeUnique<Dog>(age));
    for (auto const& a : zoo) {
        a->Speak();
    }
    ++age;
    for (auto const& a : zoo) {
        a->Speak();
    }
    UniquePtr<FILE> fp;
    fp.Reset();
    fp.Reset(fopen("a.txt", "r"));
    return 0;
}
