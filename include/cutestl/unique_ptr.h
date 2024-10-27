#pragma once

#if __cplusplus >= 202002L
#include <concepts>
#endif
#include <cstdio>
#include <iostream>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace cutestl {

using std::cout;
using std::endl;

template <typename T>
struct DefaultDeleter {
    void operator()(T* p) const { delete p; }
};

template <>
struct DefaultDeleter<FILE> {
    void operator()(FILE* p) const { fclose(p); }
};

template <typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr {
    template <typename U, typename UDeleter>
    friend class UniquePtr;

public:
    UniquePtr(std::nullptr_t dummy = nullptr) : ptr_(nullptr) {}

    explicit UniquePtr(T* ptr) : ptr_(ptr) {}

    ~UniquePtr() {
        if (ptr_) {
            Deleter{}(ptr_);
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
    UniquePtr& operator=(UniquePtr const& other) = delete;

    UniquePtr& operator=(UniquePtr&& other) {
        if (this != &other) [[likely]] {
            fclose(ptr_);
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }

    T* operator->() const { return ptr_; }

    T& operator*() const { return *ptr_; }

#if __cplusplus >= 202002L
    template <typename U, typename UDeleter>
        requires(std::convertible_to<U*, T*>)  // C++20 ->
#else
    template <typename U, typename UDeleter,
              std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>  // C++11 ->
#endif
    UniquePtr(UniquePtr<U, UDeleter> other) {
        ptr_ = std::exchange(other.ptr_, nullptr);
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

// ---------------------------------

template <typename T, typename... Args>
UniquePtr<T> MakeUnique(Args&&... args) {
    return UniquePtr<T>{new T{std::forward<Args>(args)...}};
}

}  // namespace cutestl
