#include <cutestl/unique_ptr.h>

#include <cstdio>
#include <memory>
// #include <filesystem>
// #include <fstream>
#include <functional>
#include <iostream>
#include <vector>

namespace cutestl {}

// 创建一个可调用对象的删除器（避免使用 std::function）
template <typename T>
struct DefaultDeleter {
    void operator()(T* p) const {
        std::cout << "DefaultDeleter: delete" << std::endl;
        delete p;  // 默认使用 delete 释放内存
    }
};

// DefaultDeleter 对数组类型的偏特化
template <typename T>
struct DefaultDeleter<T[]> {
    void operator()(T* p) const {
        std::cout << "DefaultDeleter: delete []" << std::endl;
        delete[] p;
    }
};

// DefaultDeleter 对文件类型的全特化
template <>
struct DefaultDeleter<FILE> {
    void operator()(FILE* p) const { fclose(p); }
};

template <typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr {
public:
    UniquePtr() : ptr_(nullptr) {}
    // NOTE: 闪回：避免非指向动态内存的指针隐式构造
    explicit UniquePtr(T* ptr) : ptr_(ptr) {}
    ~UniquePtr() {
        if (ptr_) {
            Deleter{}(ptr_);
        }
    }

    UniquePtr(UniquePtr const& other) = delete;
    UniquePtr& operator=(UniquePtr const& other) = delete;

    UniquePtr(UniquePtr&& other) {
        Deleter{}(ptr_);
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
    }

    UniquePtr& operator=(UniquePtr&& other) {
        // 判断是否自赋值
        if (this != &other) [[likely]] {
            fclose(ptr_);
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }

    // NOTE: -> 自己递归下去直到找到原始指针
    // 编译器能解引用
    T* operator->() { return ptr_; }
    T* Get() { return ptr_; }

private:
    T* ptr_ = nullptr;
};

// UniquePtr 针对 T[] 的偏特化
// NOTE: 偏特化模板参数不能有默认值
// 由于Deleter = DefaultDeleter<T>
// 则 UniquePtr<T[], Deleter> 中 T[] 传递给 DefaultDeleter
// NOTE: 这里使用继承是偷懒么？这个偏特化继承主模板
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> : public UniquePtr<T, Deleter> {};

// ---------------------------------

// template <typename T, typename... Args>
// UniquePtr<T> MakeUnique(Args... args) {
//     return UniquePtr<T>{new T(args...)};
// }

struct MyClass {
    int a, b, c;
};

int main() {
    // MyClass my_class1(1, 2, 3);
    MyClass my_class2{1, 2, 3};
    auto arr = UniquePtr<int>{new int(2)};
    // auto my_class_ptr = MakeUnique<MyClass>(1, 2, 3);
    // std::cout << my_class_ptr->a << std::endl;

    return 0;
}
