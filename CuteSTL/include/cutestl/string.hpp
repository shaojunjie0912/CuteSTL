#pragma once

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iostream>

namespace cutestl {

// NOTE: 为了兼容 C 风格字符串, 末尾带 '\0'

class String {
private:
    char* data_;       // 指向动态分配的字符数组
    size_t size_;      // 字符串中的字符数 (不包括'\0')
    size_t capacity_;  // 分配的内存容量 (不包括'\0')
public:
    String() : data_(new char[1]{'\0'}), size_(0), capacity_(0) {}
    String(char const* s) {
        size_ = std::strlen(s);
        capacity_ = size_;
        data_ = new char[capacity_ + 1];
        std::strcpy(data_, s);
    }

    ~String() noexcept {
        if (data_) {
            delete[] data_;
        }
        data_ = nullptr;
        size_ = 0;
        capacity_ = 0;
    }

public:
    void Swap(String& other) noexcept {
        using std::swap;
        swap(data_, other.data_);
        swap(size_, other.size_);
        swap(capacity_, other.capacity_);
    }

public:
    void Print() const {
        for (int i = 0; i < (int)size_; ++i) {
            std::cout << data_[i] << ' ';
        }
        std::cout << '\n';
    }
};

}  // namespace cutestl