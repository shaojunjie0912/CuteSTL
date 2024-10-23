// #include <cutestl/unique_ptr.h>

#include <concepts>
#include <cstdio>
#include <iostream>
#include <memory>
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
// template <typename T>
// struct DefaultDeleter<T[]> {
//     void operator()(T* p) const {
//         std::cout << "DefaultDeleter: delete []" << std::endl;
//         delete[] p;
//     }
// };

// DefaultDeleter 对文件类型的全特化
template <>
struct DefaultDeleter<FILE> {
    void operator()(FILE* p) const { fclose(p); }
};

template <typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr {
    // NOTE: 友元类，为了能在 std::exchange 中访问 other.ptr_
    template <typename U, typename UDeleter>
    friend class UniquePtr;

public:
    // NOTE: 为了能支持 UniquePtr = nullptr;
    // 也可以直接使用 UniquePtr.Reset() 方法
    // 这个也支持默认构造 UniquePtr()
    UniquePtr(std::nullptr_t dummy = nullptr) : ptr_(nullptr) {}

    // NOTE: 闪回：避免非指向动态内存的指针隐式构造
    explicit UniquePtr(T* ptr) : ptr_(ptr) {}

    ~UniquePtr() {
        if (ptr_) {
            Deleter{}(ptr_);  // NOTE: Delter{} 创建一个临时对象
        }
    }

public:
    UniquePtr(UniquePtr const& other) = delete;

    UniquePtr(UniquePtr&& other) {
        Deleter{}(ptr_);
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
    }

public:
    // 拷贝赋值运算符
    UniquePtr& operator=(UniquePtr const& other) = delete;

    // 移动赋值运算符
    UniquePtr& operator=(UniquePtr&& other) {
        // 判断是否自赋值
        if (this != &other) [[likely]] {
            fclose(ptr_);
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
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

    // 类型转换（支持多态）
    // NOTE: new 原始指针支持 U* -> T*
    // 这里希望支持 UniquePtr<U> -> UniquePtr<T>
    // 法一：类型转换运算符
    // operator U() const {}
    // 法二：采用构造函数的隐式转换
    template <typename U, typename UDeleter>
        requires(std::convertible_to<U*, T*>)  // 确保可以转换
    UniquePtr(UniquePtr<U, UDeleter> other) {
        ptr_ = std::exchange(other.ptr_, nullptr);  // 交换后置空
    }

public:
    T* Get() const { return ptr_; }

    T* Release() { return std::exchange(ptr_, nullptr); }

    void Reset(T* other_ptr = nullptr) {
        if (ptr_) {
            Deleter{}(ptr_);
        }
        ptr_ = other_ptr;
    }

private:
    T* ptr_ = nullptr;
};

// UniquePtr 针对 T[] 的偏特化
// NOTE: 偏特化模板参数不能有默认值
// 由于Deleter = DefaultDeleter<T>
// 则 UniquePtr<T[], Deleter> 中 T[] 传递给 DefaultDeleter
// NOTE: 这里使用继承是偷懒么？这个偏特化继承主模板
// template <typename T, typename Deleer>
// class UniquePtr<T[], Deleter> : public UniquePtr<T, Deleter> {};

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
    // return UniquePtr<T>{new T(args...)};
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
