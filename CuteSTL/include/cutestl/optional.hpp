#pragma once

#include <exception>

namespace cutestl {

// 异常处理类：继承自std::exception
struct BadOptionalAccess : public std::exception {
    BadOptionalAccess() = default;
    virtual ~BadOptionalAccess() = default;

    const char *what() const noexcept override { return "BadOptionalAccess"; }
};

// Tag 类技巧
// 用于重载构造函数，因为构造函数名永远与类名相同
// 大概就是构造函数无法有名字？
// 所以创造一个
// 这里使用 explicit 的原因：
// 使用户无法使用{}构造，只能使用全局变量 nullopt 构造
struct nullopt_t {
    explicit nullopt_t() = default;
};

inline nullopt_t nullopt;

template <typename T>
struct Optional {
    // 默认构造函数
    // 不使用 m_value() 是因为T可能是没有默认构造函数的用户自定义类型
    Optional() : m_has_value(false), m_nullopt() {}

    // Optional opt(nullopt); 构造空 Optional 对象
    // nullopt 属于 nullopt_t 类型
    // 不使用 m_value() 是因为T可能是没有默认构造函数的用户自定义类型
    Optional(nullopt_t) : m_has_value(false), m_nullopt() {};

    // 构造函数
    Optional(T value) : m_has_value(true), m_value(value) {}

    // 析构函数
    // m_nullopt 不需要析构
    ~Optional() {
        if (m_has_value) {
            m_value.~T();
        }
    }

    bool HasValue() const { return m_has_value; }

    // const版本Value()
    // 返回const引用
    // const T& Value() const {
    //     if (!m_has_value) {
    //         // 注意此处异常处理
    //         throw BadOptionalAccess();
    //     }
    //     return m_value;
    // }

    // const T&& Value() const {
    //     if (!m_has_value) {
    //         throw BadOptionalAccess();
    //     }
    //     return std::move(m_value);
    // }

    // // 非const版本Value()
    // // 返回非const引用
    // T& Value() {
    //     if (!m_has_value) {
    //         // 注意此处异常处理
    //         throw BadOptionalAccess();
    //     }
    //     return m_value;
    // }
    const T &Value() const & {
        if (!m_has_value) throw BadOptionalAccess();
        return m_value;
    }

    T &Value() & {
        if (!m_has_value) throw BadOptionalAccess();
        return m_value;
    }

    const T &&Value() const && {
        if (!m_has_value) throw BadOptionalAccess();
        return std::move(m_value);
    }

    T &&Value() && {
        if (!m_has_value) throw BadOptionalAccess();
        return std::move(m_value);
    }

private:
    bool m_has_value;
    // 使用union防止T是一个没有默认构造函数的类
    // 则无法使用Optional 的构造函数的初始化列表语句m_value()
    union {
        T m_value;
        nullopt_t m_nullopt;
    };
};

}  // namespace cutestl