#pragma once

#include <compare>   // For std::partial_ordering
#include <concepts>  // For requires clauses (C++20)
#include <cstddef>   // For std::nullptr_t, size_t
#include <memory>    // For std::default_delete
#include <utility>   // For std::move, std::forward, std::swap

namespace cutestl {

// 前向声明
template <typename T, typename Deleter = std::default_delete<T>>  // 默认参数, 下面就不需要了
class UniquePtr;

template <typename T, typename Deleter>
void swap(UniquePtr<T, Deleter>& lhs, UniquePtr<T, Deleter>& rhs) noexcept;

//==============================================================================
// 1. 主模板: UniquePtr<T, Deleter>
//==============================================================================

template <typename T, typename Deleter>
class UniquePtr {
public:
    // 类型别名
    using pointer = T*;
    using element_type = T;
    using deleter_type = Deleter;

    //--------------------------------------------------------------------------
    // 构造函数 (Constructors)
    //--------------------------------------------------------------------------

    // 默认构造
    constexpr UniquePtr() noexcept = default;

    // 从 nullptr 构造
    constexpr UniquePtr(std::nullptr_t) noexcept {}

    // 从裸指针构造
    explicit UniquePtr(pointer p) noexcept : ptr_(p) {}

    // 从裸指针和删除器构造
    UniquePtr(pointer p, const deleter_type& d) noexcept : ptr_(p), deleter_(d) {}

    UniquePtr(pointer p, deleter_type&& d) noexcept : ptr_(p), deleter_(std::move(d)) {}

    // 移动构造
    UniquePtr(UniquePtr&& other) noexcept
        : ptr_(other.release()), deleter_(std::forward<deleter_type>(other.get_deleter())) {}

    // 从可转换的 UniquePtr 类型移动构造 (例如: UniquePtr<Derived> -> UniquePtr<Base>)
    template <typename U, typename E>
        requires(std::convertible_to<U*, pointer> && !std::is_array_v<U> &&
                 std::is_convertible_v<E, deleter_type>)
    UniquePtr(UniquePtr<U, E>&& other) noexcept
        : ptr_(other.release()), deleter_(std::forward<E>(other.get_deleter())) {}

    //--------------------------------------------------------------------------
    // 析构函数 (Destructor)
    //--------------------------------------------------------------------------

    ~UniquePtr() noexcept {
        if (ptr_) {
            get_deleter()(ptr_);
        }
    }

    //--------------------------------------------------------------------------
    // 赋值运算符 (Assignment)
    //--------------------------------------------------------------------------

    // 移动赋值
    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            reset(other.release());
            get_deleter() = std::forward<deleter_type>(other.get_deleter());
        }
        return *this;
    }

    // 从可转换的 UniquePtr 类型移动赋值
    template <typename U, typename E>
        requires(std::convertible_to<U*, pointer> && !std::is_array_v<U> &&
                 std::is_assignable_v<deleter_type&, E &&>)
    UniquePtr& operator=(UniquePtr<U, E>&& other) noexcept {
        reset(other.release());
        get_deleter() = std::forward<E>(other.get_deleter());
        return *this;
    }

    // 赋为 nullptr
    UniquePtr& operator=(std::nullptr_t) noexcept {
        reset();
        return *this;
    }

    //--------------------------------------------------------------------------
    // 禁止拷贝 (No Copying)
    //--------------------------------------------------------------------------
    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    //--------------------------------------------------------------------------
    // 修饰符 (Modifiers)
    //--------------------------------------------------------------------------

    [[nodiscard]] pointer release() noexcept {
        pointer temp = ptr_;
        ptr_ = nullptr;
        return temp;
    }

    void reset(pointer p = nullptr) noexcept {
        pointer old_ptr = ptr_;
        ptr_ = p;
        if (old_ptr) {
            get_deleter()(old_ptr);
        }
    }

    void swap(UniquePtr& other) noexcept {
        std::swap(ptr_, other.ptr_);
        std::swap(deleter_, other.deleter_);
    }

    //--------------------------------------------------------------------------
    // 观察器 (Observers)
    //--------------------------------------------------------------------------

    [[nodiscard]] pointer get() const noexcept { return ptr_; }

    [[nodiscard]] deleter_type& get_deleter() noexcept { return deleter_; }

    [[nodiscard]] const deleter_type& get_deleter() const noexcept { return deleter_; }

    explicit operator bool() const noexcept { return ptr_ != nullptr; }

    //--------------------------------------------------------------------------
    // 成员访问 (Member Access)
    //--------------------------------------------------------------------------

    [[nodiscard]] element_type& operator*() const { return *get(); }

    [[nodiscard]] pointer operator->() const noexcept { return get(); }

private:
    pointer ptr_{nullptr};

    // C++20 特性: 如果 Deleter 是空类型 (如 std::default_delete),
    // 这个属性可以保证它不占用任何额外的空间。
    [[no_unique_address]] deleter_type deleter_;

    // 允许数组特化版本访问 private 成员
    template <typename T2, typename Deleter2>
    friend class UniquePtr;
};

//==============================================================================
// 2. 数组偏特化: UniquePtr<T[], Deleter>
//==============================================================================
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    using pointer = T*;
    using element_type = T;
    using deleter_type = Deleter;

    // 构造/析构/移动/赋值 与主模板逻辑类似
    constexpr UniquePtr() noexcept = default;
    constexpr UniquePtr(std::nullptr_t) noexcept {}
    explicit UniquePtr(pointer p) noexcept : ptr_(p) {}
    UniquePtr(pointer p, const deleter_type& d) noexcept : ptr_(p), deleter_(d) {}
    UniquePtr(pointer p, deleter_type&& d) noexcept : ptr_(p), deleter_(std::move(d)) {}
    UniquePtr(UniquePtr&& other) noexcept
        : ptr_(other.release()), deleter_(std::forward<deleter_type>(other.get_deleter())) {}
    ~UniquePtr() noexcept {
        if (ptr_) {
            get_deleter()(ptr_);
        }
    }

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            reset(other.release());
            get_deleter() = std::forward<deleter_type>(other.get_deleter());
        }
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) noexcept {
        reset();
        return *this;
    }

    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    // 修饰符 (Modifiers)
    [[nodiscard]] pointer release() noexcept {
        pointer temp = ptr_;
        ptr_ = nullptr;
        return temp;
    }
    void reset(pointer p = nullptr) noexcept {
        pointer old_ptr = ptr_;
        ptr_ = p;
        if (old_ptr) {
            get_deleter()(old_ptr);
        }
    }
    void swap(UniquePtr& other) noexcept {
        std::swap(ptr_, other.ptr_);
        std::swap(deleter_, other.deleter_);
    }

    // 观察器 (Observers)
    [[nodiscard]] pointer get() const noexcept { return ptr_; }
    [[nodiscard]] deleter_type& get_deleter() noexcept { return deleter_; }
    [[nodiscard]] const deleter_type& get_deleter() const noexcept { return deleter_; }
    explicit operator bool() const noexcept { return ptr_ != nullptr; }

    // **数组版本的核心区别: operator[]**
    [[nodiscard]] element_type& operator[](size_t i) const { return get()[i]; }

private:
    pointer ptr_{nullptr};
    [[no_unique_address]] deleter_type deleter_;
};

//==============================================================================
// 3. 非成员函数 (Non-member functions)
//==============================================================================
template <typename T, typename Deleter>
void swap(UniquePtr<T, Deleter>& lhs, UniquePtr<T, Deleter>& rhs) noexcept {
    lhs.swap(rhs);
}

// C++20 三向比较运算符
template <typename T1, typename D1, typename T2, typename D2>
std::common_comparison_category_t<std::compare_three_way_result_t<T1*>,
                                  std::compare_three_way_result_t<T2*>>
operator<=>(const UniquePtr<T1, D1>& lhs, const UniquePtr<T2, D2>& rhs) {
    return lhs.get() <=> rhs.get();
}

template <typename T, typename D>
bool operator==(const UniquePtr<T, D>& ptr, std::nullptr_t) noexcept {
    return !ptr;
}

// 其他比较运算符 (`!=`, `<`, `>`, `<=`, `>=`) 由 <=> 自动推导

//==============================================================================
// 4. 工厂函数 (Factory Functions)
//==============================================================================

// make_unique 用于单个对象
template <typename T, typename... Args>
    requires(!std::is_array_v<T>)
[[nodiscard]] UniquePtr<T> make_unique(Args&&... args) {
    return UniquePtr<T>(new T(std::forward<Args>(args)...));
}

// make_unique 用于数组 (值初始化)
template <typename T>
    requires(std::is_unbounded_array_v<T>)
[[nodiscard]] UniquePtr<T> make_unique(size_t size) {
    using ElementType = std::remove_extent_t<T>;
    return UniquePtr<T>(new ElementType[size]());
}

// C++20: make_unique_for_overwrite 用于单个对象 (默认初始化)
template <typename T>
    requires(!std::is_array_v<T>)
[[nodiscard]] UniquePtr<T> make_unique_for_overwrite() {
    return UniquePtr<T>(new T);
}

// C++20: make_unique_for_overwrite 用于数组 (默认初始化)
template <typename T>
    requires(std::is_unbounded_array_v<T>)
[[nodiscard]] UniquePtr<T> make_unique_for_overwrite(size_t size) {
    using ElementType = std::remove_extent_t<T>;
    return UniquePtr<T>(new ElementType[size]);
}

}  // namespace cutestl
