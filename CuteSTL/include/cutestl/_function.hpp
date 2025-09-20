#pragma once

#include <functional>
#include <memory>
#include <type_traits>
#include <typeinfo>
#include <utility>

// 参考:
// - https://github.com/parallel101/stl1weekend/blob/main/_Function.hpp
// - https://github.com/parallel101/stl1weekend/blob/main/test_Function.cpp

namespace cutestl {

// 通用模板, 检测不符合 RT(Args...) 模式
template <typename FnSig>
class Function {
    static_assert(!std::is_same_v<FnSig, FnSig>, "函数签名无效!");
};

// NOTE: Function 偏特化
template <typename RT, typename... Args>
class Function<RT(Args...)> {
private:
    class FuncBase {
    public:
        virtual RT Call(Args... args) = 0;                    // 类型擦除后的统一接口
        virtual std::unique_ptr<FuncBase> Clone() const = 0;  // 值语义, 克隆当前函数对象
        virtual ~FuncBase() = default;
    };

    template <typename Fn>
    class FuncImpl : public FuncBase {
    private:
        Fn f_;  // 仿函数对象

    public:
        // HACK:
        // - 直接传一个仿函数对象
        // - 或者传 (std::in_place, args..) 就地构造 Fn f_ 对象
        template <typename... CArgs>
        explicit FuncImpl(std::in_place_t, CArgs&&... args) : f_(std::forward<CArgs>(args)...) {}

        RT Call(Args... args) override { return std::invoke(f_, std::forward<Args>(args)...); }

        std::unique_ptr<FuncBase> Clone() const override {
            // HACK: 这里也是就地构造 f_ 作为 FuncImpl 构造函数形参
            return std::make_unique<FuncImpl>(std::in_place, f_);
        }
    };

    std::unique_ptr<FuncBase> base_;

public:
    Function() = default;

    // TODO: 这里 std::decay_t 一定要加么？
    // HACK: 不加 explicit 为了 lambda 能隐式转换为 Function
    template <typename Fn,
              std::enable_if_t<std::is_invocable_r_v<RT, std::decay_t<Fn>, Args...> &&
                                   std::is_copy_constructible_v<Fn> &&
                                   !std::is_same_v<std::decay_t<Fn>, Function<RT(Args...)>>,
                               int> = 0>
    Function(Fn&& f)
        : base_(std::make_unique<FuncImpl<std::decay_t<Fn>>>(std::in_place, std::forward<Fn>(f))) {}

    Function(Function const& other) {  // 拷贝构造
        if (!other.base_) {
            base_ = nullptr;
        } else {
            base_ = other.base_->Clone();
        }
    }

    Function& operator=(Function const& other) {  // 拷贝赋值
        if (this == &other) {
            return *this;
        }
        if (!other.base_) {
            base_ = nullptr;
        } else {
            base_ = other.base_->Clone();
        }
        return *this;
    }
    Function(Function&&) = default;             // 默认移动构造
    Function& operator=(Function&&) = default;  // 默认移动赋值

    operator bool() const noexcept { return base_ != nullptr; }

    // 这里不加&& 因为不是模板<->万能引用
    // 为什么这里能使用 std::forward?
    RT operator()(Args... args) const {
        // HACK: 如果仿函数对象指针为空则报错
        if (!base_) {
            throw std::bad_function_call();
        }
        return base_->Call(std::forward<Args>(args)...);
    }
};

}  // namespace cutestl
