#include <cutestl/unique_ptr.h>

#include <cstdio>
#include <memory>
// #include <filesystem>
// #include <fstream>
#include <functional>
#include <iostream>
#include <vector>

namespace cutestl {}

// 创建一个函数对象的删除器（避免使用 std::function）
template <typename T>
struct DefaultDeleter {
    void operator()(T* p) const {
        delete p;  // 默认使用 delete 释放内存
    }
};

// 对文件类型的偏特化
template <>
struct DefaultDeleter<FILE> {
    void operator()(FILE* p) const { fclose(p); }
};

template <typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr {
public:
    UniquePtr() : ptr_(nullptr) {}
    UniquePtr(T* ptr, Deleter deleter = DefaultDeleter<T>{}) : ptr_(ptr), deleter_(deleter) {}
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

    T* Get() { return ptr_; }

private:
    T* ptr_ = nullptr;
    Deleter deleter_;
};

int main() {
    auto a{UniquePtr<FILE>{new FILE}};
    // auto a{UniquePtr<FILE>{fopen("build/tests/a.txt", "r")}};
    // auto b{UniquePtr<FILE>{fopen("build/tests/b.txt", "r")}};
    // std::cout << (char)fgetc(a.Get()) << std::endl;
    return 0;
}
