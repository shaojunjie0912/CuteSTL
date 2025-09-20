#pragma once

#include <algorithm>  // for std::swap, std::min, std::max
#include <cstring>    // for std::strlen, std::memcpy, std::strcmp
#include <iostream>   // for std::istream, std::ostream
#include <utility>    // for std::move

class String {
public:
    // 1. 构造函数与析构函数 (Constructors & Destructor)

    // 默认构造函数：创建一个空字符串
    String() noexcept = default;

    // C风格字符串构造函数
    // explicit关键字防止从 "const char*" 到 String 的隐式转换
    explicit String(const char* s) {
        if (s == nullptr) {
            return;  // 保持默认构造状态
        }
        size_ = std::strlen(s);
        capacity_ = size_;
        data_ = new char[capacity_ + 1];  // 为字符串内容和末尾的 '\0' 分配空间
        std::memcpy(data_, s, size_);
        data_[size_] = '\0';
    }

    // 析构函数：释放动态分配的内存
    ~String() { delete[] data_; }

    // 拷贝构造函数：执行深拷贝
    String(const String& other) : size_(other.size_), capacity_(other.capacity_) {
        // 仅在other拥有数据时才分配内存
        if (other.data_) {
            data_ = new char[capacity_ + 1];
            std::memcpy(data_, other.data_, size_ + 1);  // 包含'\0'
        }
    }

    // 移动构造函数：从右值“窃取”资源，避免拷贝
    // noexcept 关键字对于标准库容器的性能优化至关重要
    String(String&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        // 将源对象置于一个有效的、可析构的状态
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    // 2. 赋值运算符 (Assignment Operators) - 采用 Copy-and-Swap 惯用法

    // 统一赋值运算符 (pass-by-value)
    // 结合了拷贝赋值和移动赋值的功能
    String& operator=(String other) noexcept {
        swap(other);
        return *this;
    }

    // 3. 容量相关 (Capacity)

    // 返回字符串中的字符数
    std::size_t size() const noexcept { return size_; }
    std::size_t length() const noexcept { return size_; }

    // 返回当前分配的存储空间大小
    std::size_t capacity() const noexcept { return capacity_; }

    // 检查字符串是否为空
    bool empty() const noexcept { return size_ == 0; }

    // 4. 元素访问 (Element Access)

    // 返回对指定位置字符的引用（带边界检查）
    char& at(std::size_t pos) {
        if (pos >= size_) {
            throw std::out_of_range("MyString::at");
        }
        return data_[pos];
    }

    const char& at(std::size_t pos) const {
        if (pos >= size_) {
            throw std::out_of_range("MyString::at");
        }
        return data_[pos];
    }

    // 返回对指定位置字符的引用（不带边界检查）
    char& operator[](std::size_t pos) noexcept { return data_[pos]; }

    const char& operator[](std::size_t pos) const noexcept { return data_[pos]; }

    // 5. C风格字符串接口 (C-style String Interface)

    // 返回一个指向以空字符结尾的C风格字符串的指针
    const char* c_str() const noexcept { return data_ ? data_ : ""; }

    // 6. 修改器 (Modifiers)

    // 拼接另一个MyString
    String& operator+=(const String& rhs) {
        if (rhs.size_ == 0) {
            return *this;
        }
        // 检查容量是否足够
        if (size_ + rhs.size_ > capacity_) {
            // 扩容策略：通常是2倍增长，或至少满足所需大小
            std::size_t new_capacity = std::max(capacity_ * 2, size_ + rhs.size_);
            reserve(new_capacity);
        }
        // 拷贝数据
        std::memcpy(data_ + size_, rhs.data_, rhs.size_);
        size_ += rhs.size_;
        data_[size_] = '\0';
        return *this;
    }

    // 拼接C风格字符串
    String& operator+=(const char* rhs) { return *this += String(rhs); }

    // 清空字符串内容
    void clear() noexcept {
        if (data_) {
            size_ = 0;
            data_[0] = '\0';
        }
    }

    // 交换两个MyString对象的内容
    void swap(String& other) noexcept {
        using std::swap;
        swap(data_, other.data_);
        swap(size_, other.size_);
        swap(capacity_, other.capacity_);
    }

private:
    // 预留存储空间
    void reserve(std::size_t new_capacity) {
        if (new_capacity <= capacity_) return;

        char* new_data = new char[new_capacity + 1];
        // 如果原来有数据，则拷贝过来
        if (data_) {
            std::memcpy(new_data, data_, size_ + 1);
        } else {
            // 如果原来是空字符串，确保新数据是有效的空字符串
            new_data[0] = '\0';
        }
        delete[] data_;
        data_ = new_data;
        capacity_ = new_capacity;
    }

    char* data_ = nullptr;      // 指向动态分配的字符数组
    std::size_t size_ = 0;      // 字符串当前长度 (不包括'\0')
    std::size_t capacity_ = 0;  // 当前分配的内存容量 (不包括'\0')
};

// 7. 非成员函数重载 (Non-member Function Overloads)

// 字符串拼接
inline String operator+(String lhs, const String& rhs) {
    lhs += rhs;
    return lhs;
}

inline String operator+(String lhs, const char* rhs) {
    lhs += rhs;
    return lhs;
}

inline String operator+(const char* lhs, String rhs) {
    // 为了复用，创建一个临时的MyString对象，然后与rhs拼接
    String temp(lhs);
    temp += rhs;
    return temp;
}

// 关系运算符
inline bool operator==(const String& lhs, const String& rhs) {
    return (lhs.size() == rhs.size()) && (std::strcmp(lhs.c_str(), rhs.c_str()) == 0);
}

inline bool operator!=(const String& lhs, const String& rhs) { return !(lhs == rhs); }

inline bool operator<(const String& lhs, const String& rhs) {
    return std::strcmp(lhs.c_str(), rhs.c_str()) < 0;
}

inline bool operator<=(const String& lhs, const String& rhs) { return !(rhs < lhs); }

inline bool operator>(const String& lhs, const String& rhs) { return rhs < lhs; }

inline bool operator>=(const String& lhs, const String& rhs) { return !(lhs < rhs); }

// 流插入运算符
inline std::ostream& operator<<(std::ostream& os, const String& s) {
    os << s.c_str();
    return os;
}

// 非成员swap函数
inline void swap(String& a, String& b) noexcept { a.swap(b); }
