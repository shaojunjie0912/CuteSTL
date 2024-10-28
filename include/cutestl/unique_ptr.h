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

using std::cout;
using std::endl;

namespace cutestl {

template <typename T>
struct DefaultDeleter {
    void operator()(T* p) const { delete p; }
};

template <typename T>
struct DefaultDeleter<T[]> {
    void operator()(T* p) const { delete[] p; }
};

template <typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr {
private:
    T* ptr_ = nullptr;  // 原始指针
    Deleter deleter_;   // 删除器

    template <typename U, typename UDeleter>
    friend class UniquePtr;

public:
    UniquePtr(std::nullptr_t dummy = nullptr) noexcept : ptr_(nullptr) {}

    explicit UniquePtr(T* ptr) noexcept : ptr_(ptr) {}

    ~UniquePtr() noexcept {
        if (ptr_) {
            deleter_(ptr_);
        }
    }

public:
    UniquePtr(UniquePtr const& other) = delete;

    UniquePtr(UniquePtr&& other) : ptr_(other.ptr_) { other.ptr_ = nullptr; }

public:
    UniquePtr& operator=(UniquePtr const& other) = delete;

    UniquePtr& operator=(UniquePtr&& other) {
        if (this != &other) [[likely]] {
            if (ptr_) {
                deleter_(ptr_);
            }
            std::exchange(other.ptr_, nullptr);
        }
        return *this;
    }

    T* operator->() const { return ptr_; }

    T& operator*() const { return *ptr_; }

    // std::add_lvalue_reference_t<T> operator*() const noexcept { return *ptr_; }

    explicit operator bool() const noexcept { return ptr_ != nullptr; }

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

template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> : public UniquePtr<T, Deleter> {
    using UniquePtr<T, Deleter>::UniquePtr;  // HACK: 继承构造函数

    std::add_lvalue_reference_t<T> operator[](std::size_t i) {
        return this->Get()[i];  // HACK: 此处用 this
    }
};

// ---------------------------------

template <typename T, typename... Args>
UniquePtr<T> MakeUnique(Args&&... args) {
    return UniquePtr<T>{new T{std::forward<Args>(args)...}};
}

}  // namespace cutestl
