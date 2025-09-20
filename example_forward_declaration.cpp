// 演示前向声明的重要性

#include <iostream>

// ===== 错误示例：没有前向声明 =====
/*
// 如果没有前向声明，这样写会编译错误：
template<typename T, typename D>
void some_function(MyUniquePtr<T, D>& ptr);  // 错误：MyUniquePtr 未定义

template<typename T, typename D = DefaultDeleter<T>>  // 这里有默认参数
class MyUniquePtr {
    // ...
};

template<typename T, typename D = DefaultDeleter<T>>  // 错误：重复的默认参数
class MyUniquePtr {
    // ...
};
*/

// ===== 正确示例：使用前向声明 =====

// 1. 前向声明（只在这里指定默认参数）
template <typename T, typename D = std::default_delete<T>>
class MyUniquePtr;

// 2. 现在可以声明使用该类型的函数
template <typename T, typename D>
void some_function(MyUniquePtr<T, D>& ptr);

template <typename T, typename D>
void swap(MyUniquePtr<T, D>& lhs, MyUniquePtr<T, D>& rhs);

// 3. 实际的类定义（不能再指定默认参数）
template <typename T, typename D>
class MyUniquePtr {
private:
    T* ptr_;
    D deleter_;

public:
    explicit MyUniquePtr(T* p = nullptr) : ptr_(p) {}

    ~MyUniquePtr() {
        if (ptr_) {
            deleter_(ptr_);
        }
    }

    // 移动构造
    MyUniquePtr(MyUniquePtr&& other) noexcept
        : ptr_(other.ptr_), deleter_(std::move(other.deleter_)) {
        other.ptr_ = nullptr;
    }

    T* get() const { return ptr_; }
    T* release() {
        T* temp = ptr_;
        ptr_ = nullptr;
        return temp;
    }

    void swap(MyUniquePtr& other) {
        std::swap(ptr_, other.ptr_);
        std::swap(deleter_, other.deleter_);
    }
};

// 4. 实现前面声明的函数
template <typename T, typename D>
void swap(MyUniquePtr<T, D>& lhs, MyUniquePtr<T, D>& rhs) {
    lhs.swap(rhs);
}

template <typename T, typename D>
void some_function(MyUniquePtr<T, D>& ptr) {
    std::cout << "处理指针: " << ptr.get() << std::endl;
}

// 5. 使用示例
int main() {
    // 注意：这里没有指定第二个模板参数，使用了默认的 std::default_delete<int>
    MyUniquePtr<int> ptr1(new int(42));

    // 显式指定删除器
    MyUniquePtr<int, std::default_delete<int>> ptr2(new int(24));

    some_function(ptr1);
    some_function(ptr2);

    return 0;
}
