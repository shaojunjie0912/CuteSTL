#pragma once

#include <functional>
#include <memory>
#include <type_traits>
#include <typeinfo>
#include <utility>

// 参考实现: https://github.com/parallel101/stl1weekend/blob/main/_Function.hpp

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
        virtual std::unique_ptr<FuncBase> Clone() const = 0;  // 原型模式，克隆当前函数对象
        virtual std::type_info const& Type() const = 0;       // 获得函数对象类型信息
        virtual ~FuncBase() = default;
    };

    template <typename Fn>
    class FuncImpl : public FuncBase {
    private:
        Fn f_;  // 仿函数对象

    public:
        // HACK: 其实就只传了一个仿函数对象..orz
        // 为啥还要用可变参数
        // 可能还会传 (std::in_place, args..) 就地构造
        template <typename... CArgs>
        explicit FuncImpl(std::in_place_t, CArgs&&... args) : f_(std::forward<Args>(args)...) {}

        RT Call(Args&&... args) override { return std::invoke(f_, std::forward<Args>(args)...); }

        std::unique_ptr<FuncBase> Clone() const override {
            // HACK: 这里也是就地构造 f_ 作为 FuncImpl 构造函数形参
            return std::make_unique<FuncImpl>(std::in_place, f_);
        }

        // TODO: typeid
        std::type_info const& Type() const override { return typeid(Fn); }
    };

    std::unique_ptr<FuncBase> base_;

public:
    Function() = default;

    template <typename Fn, std::enable_if_t<std::is_invocable_r_v<RT, Fn, Args...> &&
                                                std::is_copy_constructible_v<Fn>,
                                            int> = 0>
    Function(Fn&& f) : base_(std::make_unique<FuncImpl<Fn>>(std::forward<Fn>(f))) {}
};

}  // namespace cutestl
