#pragma once

#include <fmt/core.h>

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <utility>

// [ ]: 思考: std::uninitialized_fill & copy > std::copy

namespace cutestl {

template <typename T>
class Vector {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type;
    using iterator = value_type*;
    using const_iterator = const iterator;

private:
    value_type* data_;
    size_type size_;
    size_type capacity_;

public:
    value_type* Allocate(size_type n) {
        return static_cast<value_type*>(operator new(n * sizeof(value_type)));
    }
    void Deallocate(value_type* p) { operator delete(p); }

public:
    Vector() : data_(nullptr), size_(0), capacity_(0) {}

    Vector(size_type n, T val) : data_(Allocate(n)), size_(n), capacity_(n) {
        // 1. 分配内存
        // 2. 构造对象
        std::uninitialized_fill_n(data_, size_, val);
    }

    template <typename InputIt>
    Vector(InputIt first, InputIt last) {
        size_type n = std::distance(first, last);
        data_ = Allocate(n);
        std::uninitialized_copy(first, last, data_);
        size_ = n;
        capacity_ = n;
    }

    Vector(std::initializer_list<T> init_list)
        : data_(new T[init_list.size()]), size_(init_list.size()) {
        auto tmp{data_};
        for (auto& val : init_list) {
            *(tmp++) = std::move(val);
        }
    }

    // NOTE: 使用委托构造简略写法
    Vector(Vector const& other) : Vector(other.begin(), other.end()) { fmt::println("拷贝构造"); }

    Vector& operator=(Vector const& other) {
        fmt::println("拷贝赋值");
        if (this != &other) {
            if (other.size_ > capacity_) {
                Vector tmp{other};
                this->Swap(tmp);  // [ ]: Maybe 是高效 Swap?
            } else {
            }
        }
        return *this;
    }

    Vector(Vector&& other) noexcept {
        fmt::println("移动构造");
        data_ = other.data_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    Vector& operator=(Vector&& other) noexcept {
        fmt::println("移动赋值");
        if (this != &other) {
            this->~Vector();
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    // [ ]: 为什么析构函数必须是 noexcept?
    // -> 如果出现异常则递归析构
    ~Vector() noexcept {
        if (data_) {
            std::destroy(begin(), end());  // 1. 析构对象
            Deallocate(data_);             // 2. 释放内存
        }
    }

public:
    // T& operator[](size_type index) { return *(data_ + index); }
    // T const& operator[](size_type index) const { return *(data_ + index); }

public:
    void Clear() {
        delete[] data_;
        data_ = nullptr;
        size_ = 0;
    }

    void Swap(Vector& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void Reserve(size_type n) {
        if (capacity_ >= n) {
            return;
        }
        iterator new_data{Allocate(n)};
        std::uninitialized_copy(begin(), end(), new_data);
        std::destroy(begin(), end());
        Deallocate(data_);
        data_ = new_data;
        capacity_ = n;
    }

public:
    size_type size() const { return size_; }

    value_type* data() { return data_; }

    size_type capacity() const { return capacity_; }

    iterator begin() { return data_; }

    const_iterator begin() const { return data_; }

    iterator end() { return data_ + size_; }

    const_iterator end() const { return data_ + size_; }

public:
    void Print() const {
        fmt::print("Vec = [ ", size_);
        for (auto const& val : *this) {
            fmt::print("{} ", val);
        }
        fmt::print("] ");
        fmt::print("Size = {} ", size_);
        fmt::print("Capacity = {} ", capacity_);
        fmt::print("\n");
    }
};

}  // namespace cutestl