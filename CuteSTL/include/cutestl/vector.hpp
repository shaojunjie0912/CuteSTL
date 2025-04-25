#pragma once

#include <fmt/core.h>

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <utility>

#include "allocator.hpp"

// [ ]: 思考: std::uninitialized_fill & copy > std::copy
// NOTE: std::construct_at C++20 == placement new

namespace cutestl {

template <typename T, typename Alloc = Allocator<T>>
class Vector {
public:
    using value_type = T;
    using allocator_type = Alloc;
    using size_type = std::size_t;
    using pointer = T*;
    using const_pointer = T const*;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = T const&;
    using iterator = T*;
    using const_iterator = T const*;
    using reverse_iterator = std::reverse_iterator<T*>;
    using const_reverse_iterator = std::reverse_iterator<T const*>;

private:
    value_type* start_;           // 指向第一个元素的指针
    value_type* finish_;          // 指向最后一个元素的下一个位置
    value_type* end_of_storage_;  // 指向内存空间的尾后位置

public:
    constexpr Vector() : start_(nullptr), finish_(nullptr), end_of_storage_(nullptr) {}

    constexpr Vector(size_type n, T val)
        : start_(Alloc::Allocate(n)), finish_(start_ + n), end_of_storage_(start_ + n) {
        // 1. 分配内存
        // 2. 构造对象
        std::uninitialized_fill(start_, finish_, val);
    }

    template <typename InputIt>
    Vector(InputIt first, InputIt last) {
        size_type n = std::distance(first, last);
        start_ = Alloc::Allocate(n);
        std::uninitialized_copy(first, last, start_);
        finish_ = start_ + n;
        end_of_storage_ = start_ + n;
    }

    Vector(std::initializer_list<T> init_list) : Vector(init_list.begin(), init_list.end()) {}

    ~Vector() noexcept {
        if (start_) {
            std::destroy(start_, finish_);  // 1. 调用每个元素的析构函数
            Alloc::Deallocate(start_);      // 2. 释放 start_ 指向的内存
        }
    }

public:
    // NOTE: 使用委托构造简略写法
    Vector(Vector const& other) : Vector(other.begin(), other.end()) { fmt::println("拷贝构造"); }

    Vector(Vector&& other) noexcept {
        fmt::println("移动构造");
        start_ = other.start_;
        finish_ = other.finish_;
        end_of_storage_ = other.end_of_storage_;
        other.start_ = nullptr;
        other.finish_ = nullptr;
        other.end_of_storage_ = nullptr;
    }

public:
    Vector& operator=(Vector const& other) {
        fmt::println("拷贝赋值");
        if (this != &other) {
            // 1. 如果 other.size > capacity 则重新分配内存
            if (other.Size() > Capacity()) {
                Vector tmp{other};  // tmp 离开作用域自动析构释放内存
                this->Swap(tmp);    // [ ]: Maybe 是高效 Swap?
            }
            // 2. 如果 other.size <= capacity 则在当前内存上操作
            else {
                if (other.Size() > Size()) {
                    std::copy(other.start_, other.start_ + Size(), start_);
                    finish_ = std::uninitialized_copy(other.start_ + Size(), other.finish_, finish_);
                } else {
                    iterator new_finish{std::copy(other.start_, other.finish_, start_)};
                    std::destroy(start_ + other.Size(), finish_);
                    finish_ = new_finish;
                }
            }
        }
        return *this;
    }

    Vector& operator=(Vector&& other) noexcept {
        fmt::println("移动赋值");
        if (this != &other) {
            std::destroy(begin(), end());
            Alloc::Deallocate(start_);
            start_ = other.start_;
            finish_ = other.finish_;
            end_of_storage_ = other.end_of_storage_;
            other.start_ = nullptr;
            other.finish_ = nullptr;
            other.end_of_storage_ = nullptr;
        }
        return *this;
    }

    reference operator[](size_type n) { return *(start_ + n); }

    const_reference operator[](size_type n) const { return *(start_ + n); }

public:
    size_type Size() const { return finish_ - start_; }

    value_type* Data() { return start_; }

    size_type Capacity() const { return end_of_storage_ - start_; }

public:
    reference Front() { return *start_; }

    const_reference Front() const { return *start_; }

    reference Back() { return *(finish_ - 1); }

    const_reference Back() const { return *(finish_ - 1); }

    bool Empty() const { return start_ == finish_; }

    void Clear() {
        std::destroy(begin(), end());
        finish_ = start_;
    }

    void Swap(Vector& other) noexcept {
        // NOTE: std::swap 内部是移动
        std::swap(start_, other.start_);
        std::swap(finish_, other.finish_);
        std::swap(end_of_storage_, other.end_of_storage_);
    }

    void Reserve(size_type n) {
        if (Capacity() >= n) {
            return;
        }
        iterator new_start{Alloc::Allocate(n)};
        iterator new_finish{std::uninitialized_copy(begin(), end(), new_start)};
        std::destroy(start_, finish_);
        Alloc::Deallocate(start_);
        start_ = new_start;
        finish_ = new_finish;
        end_of_storage_ = start_ + n;
    }

public:
    iterator Insert(const iterator pos, value_type const& val) {
        if (end_of_storage_ - finish_ > 0) {
            if (pos != finish_) {
                std::construct_at(finish_, *(finish_ - 1));  // NOTE: 需要构造最后一个空位
                std::move_backward(pos, finish_ - 1, finish_);
                *pos = val;
                ++finish_;
                return pos;
            } else {
                std::construct_at(finish_, val);
                ++finish_;
                return pos;
            }
        } else {
            size_type new_size = std::max(2 * Size(), Size() + 1);
            iterator new_start{Alloc::Allocate(new_size)};
            iterator new_finish{std::uninitialized_move(start_, pos, new_start)};
            iterator ret{new_finish};
            std::construct_at(new_finish, val);
            ++new_finish;
            new_finish = std::uninitialized_move(pos, finish_, new_finish);
            std::destroy(start_, finish_);
            Alloc::Deallocate(start_);
            start_ = new_start;
            finish_ = new_finish;
            end_of_storage_ = start_ + new_size;
            return ret;
        }
    }

    iterator Insert(iterator pos, value_type&& val) {
        // 1. capacity 足够
        if (end_of_storage_ - finish_ > 0) {
            if (pos != finish_) {
                std::construct_at(finish_, std::move(*(finish_ - 1)));  // 最后一个元素右移, 腾出一个空位
                std::move_backward(pos, finish_ - 1, finish_);          // 移动剩余元素
                *pos = std::move(val);
                ++finish_;
                return pos;
            } else {
                std::construct_at(finish_, std::move(val));
                ++finish_;
                return pos;
            }
        }
        // 2. 没有 capacity
        else {
            size_type new_size = std::max(2 * Size(), Size() + 1);
            iterator new_start{Alloc::Allocate(new_size)};
            iterator new_finish{std::uninitialized_move(start_, pos, new_start)};
            iterator ret{new_finish};
            std::construct_at(new_finish, std::move(val));
            ++new_finish;
            new_finish = std::uninitialized_move(pos, finish_, new_finish);
            std::destroy(start_, finish_);
            Alloc::Deallocate(start_);
            start_ = new_start;
            finish_ = new_finish;
            end_of_storage_ = start_ + new_size;
            return ret;
        }
    }

    // ⭐ 核心! ️
    iterator Insert(iterator pos, size_type n, value_type const& val) {
        if (n == 0) {
            return pos;
        }
        if (end_of_storage_ - finish_ > n) {
            size_type elemts_after = finish_ - pos;  // pos 和 end() 之间的元素个数
            if (elemts_after > n) {                  // 切分原数组
                std::uninitialized_move(finish_ - n, finish_, finish_);
                std::move_backward(pos, finish_ - n, finish_);  // NOTE: 从后往前移动, 防止重叠
                std::fill_n(pos, n, val);
                finish_ += n;
                return pos;
            } else {  // 切分插入数组
                std::uninitialized_fill(finish_, pos + n, val);
                std::uninitialized_move(pos, finish_, pos + n);
                std::fill(pos, finish_, val);
                finish_ += n;
                return pos;
            }
        } else {
            // NOTE: MSVC: * 2; GCC: * 1.5
            size_type new_size = std::max(2 * Size(), Size() + n);
            iterator new_start{Alloc::Allocate(new_size)};
            iterator new_finish{std::uninitialized_move(start_, pos, new_start)};  // 1. 移动左边部分
            iterator ret{new_finish};
            new_finish = std::uninitialized_fill_n(new_finish, n, val);      // 2. 填充中间部分
            new_finish = std::uninitialized_move(pos, finish_, new_finish);  // 3. 移动右边部分
            std::destroy(start_, finish_);
            Alloc::Deallocate(start_);
            start_ = new_start;
            finish_ = new_finish;
            end_of_storage_ = start_ + new_size;
            return ret;
        }
    }

    template <typename... Args>
    iterator Emplace(iterator pos, Args&&... args) {
        return Insert(pos, value_type{std::forward<Args>(args)...});
    }

    template <typename... Args>
    iterator EmplaceBack(Args&&... args) {
        return Insert(finish_, value_type{std::forward<Args>(args)...});
    }

    iterator Erase(iterator pos) { return Erase(pos, pos + 1); }

    iterator Erase(iterator first, iterator last) {
        iterator new_finish{std::move(last, finish_, first)};
        std::destroy(new_finish, finish_);
        finish_ = new_finish;
        return first;
    }

    void PushBack(value_type const& val) { Insert(finish_, val); }

    void PushBack(value_type&& val) { Insert(finish_, std::move(val)); }

    void PopBack() { finish_ = Erase(finish_ - 1); }

public:
    iterator begin() { return start_; }

    const_iterator begin() const { return start_; }

    iterator end() { return finish_; }

    const_iterator end() const { return finish_; }

public:
    void Print() const {
        fmt::print("Vec = [ ");
        for (auto const& val : *this) {
            fmt::print("{} ", val);
        }
        fmt::print("] ");
        fmt::print("Size = {} ", Size());
        fmt::print("Capacity = {} ", Capacity());
        fmt::print("\n");
    }
};

}  // namespace cutestl