#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace cutestl {

template <typename T>
struct Vector {
private:
    T* data_;
    std::size_t size_;
    std::size_t capacity_;

public:
    Vector() : data_(nullptr), size_(0), capacity_(0) {}

    explicit Vector(std::size_t size) : data_(new T[size]), size_(size), capacity_(size) {}

    Vector(std::size_t size, T value) {
        data_ = new T[size];
        for (std::size_t i{0}; i < size; ++i) {
            data_[i] = value;
        }
    }

    Vector(std::initializer_list<T> ini_list)
        : data_(new T[ini_list.size()]), size_(ini_list.size()) {
        T* begin = data_;
        for (auto const& val : ini_list) {
            *begin = val;
            begin += sizeof(T);
        }
    }

    Vector(Vector const& other) {
        data_ = new T[other.size_];
        size_ = other.size_;
        memcpy(data_, other.data_, size_ * sizeof(T));
    }

    Vector& operator=(Vector const& other) {
        if (this == &other) {
            return *this;
        }
        if (data_) {
            delete[] data_;
        }
        size_ = other.size_;
        if (size_ != 0) {
            data_ = new T[other.size_];
            memcpy(data_, other.data_, size_ * sizeof(T));
        }
        return *this;
    }

    Vector(Vector&& other) {
        data_ = std::exchange(other.data_, nullptr);
        size_ = std::exchange(other.size_, 0);
    }

    Vector& operator=(Vector&& other) {
        if (this == &other) {
            return *this;
        }
        if (data_) {
            delete[] data_;
        }
        data_ = std::exchange(other.data_, nullptr);
        size_ = std::exchange(other.size_, 0);
        return *this;
    }

    ~Vector() { delete[] data_; }

public:
    T& operator[](std::size_t i) {
        // FIXME: 为什么不对
        // return *(data_ + i * sizeof(T));
        return data_[i];
    }

    T const& operator[](std::size_t i) const {
        // return *(data_ + i * sizeof(T));
        return data_[i];
    }

public:
    T* Begin() { return data_; }

    T* End() { return data_ + size_ * sizeof(T); }

    std::size_t Size() const noexcept { return size_; }

    std::size_t Capacity() const noexcept { return capacity_; }

    void Reserve(std::size_t size) {}

    void ShrinkToFit() {}

    void Resize(std::size_t n) {
        if (n == 0) {
            delete[] data_;
            data_ = nullptr;
            size_ = 0;
        } else {
            T* data_tmp = data_;
            std::size_t size_tmp = size_;
            data_ = new T[n]{};  // 括号初始化(对于内置类型默认初始化即零初始化)
            size_ = n;
            if (data_tmp) {
                memcpy(data_, data_tmp, std::min(size_tmp, n) * sizeof(T));
                delete[] data_tmp;
            }
        }
    }

    void Clear() {
        delete[] data_;
        data_ = nullptr;
        size_ = 0;
    }

    T const& Front() const { return data_[0]; }
    T& Front() { return data_[0]; }

    T const& Back() const { return data_[size_ - 1]; }
    T& Back() { return data_[size_ - 1]; }

    void PushBack(T val) {
        Resize(size_ + 1);
        Back() = std::move(val);
    }

    void Erase(std::size_t i) {
        for (std::size_t j = i; j < size_ - 1; ++j) {
            data_[j] = std::move(data_[j + 1]);
        }
        Resize(size_ - 1);
    }
};

}  // namespace cutestl