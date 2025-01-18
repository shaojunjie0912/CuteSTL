#pragma once

#include <fmt/core.h>

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <utility>

namespace cutestl {

template <typename T>
class Vector {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = value_type const&;
    using iterator = value_type*;
    using const_iterator = iterator const*;

private:
    T* data_;
    size_type size_;
    size_type capacity_;

public:
    T* Allocate(size_type n) { return static_cast<T*>(operator new(n * sizeof(value_type))); }
    void Deallocate(T* p) { operator delete(p); }

public:
    explicit Vector(size_type size) : data_(new T[size]{}), size_(size) {}

    Vector(size_type size, T init_val) : data_(new T[size]), size_(size) {
        for (size_type i{0}; i < size; ++i) {
            data_[i] = std::move(init_val);
        }
    }

    Vector(std::initializer_list<T> init_list)
        : data_(new T[init_list.size()]), size_(init_list.size()) {
        auto tmp{data_};
        for (auto&& val : init_list) {
            *(tmp++) = std::move(val);
        }
    }

    Vector(Vector const& other) : data_(new T[other.size_]), size_(other.size_) {
        fmt::println("拷贝构造");
        std::copy(other.data_, other.data_ + size_, data_);
    }

    Vector& operator=(Vector const& other) {
        fmt::println("拷贝赋值");
        if (this != &other) {
            this->~Vector();
            data_ = new T[other.size_];
            size_ = other.size_;
            std::copy(other.data_, other.data_ + size_, data_);
        }
        return *this;
    }

    Vector(Vector&& other) noexcept {
        fmt::println("移动构造");
        data_ = other.data_;
        size_ = other.size_;
        other.data_ = nullptr;
        other.size_ = 0;
    }

    Vector& operator=(Vector&& other) noexcept {
        fmt::println("移动赋值");
        if (this != &other) {
            this->~Vector();
            data_ = other.data_;
            size_ = other.size_;
            other.data_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    ~Vector() {
        if (data_) {
            delete[] data_;
            data_ = nullptr;
        }
    }

public:
    T& operator[](size_type index) { return *(data_ + index); }
    T const& operator[](size_type index) const { return *(data_ + index); }

public:
    size_type Size() const { return size_; }
    void Clear() {
        delete[] data_;
        data_ = nullptr;
        size_ = 0;
    }
    T* Data() { return data_; }

public:
    void Print() const {
        fmt::print("Vec[{}]: [ ", size_);
        for (size_type i{0}; i < size_; ++i) {
            fmt::print("{}, ", *(data_ + i));
        }
        fmt::print("]\n");
    }
};

}  // namespace cutestl