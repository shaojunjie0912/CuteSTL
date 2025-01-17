#include <cutestl/functional.hpp>
#include <iostream>
#include <memory>
#include <print>
#include <string>

using namespace cutestl;

int SimpleFunc(int a, int b) {
    printf("[SimpleFunc] a = %d, b = %d\n", a, b);
    return a + b;
}

struct Foo {
    void operator()(int a, int b) { printf("[ClassOperator] a = %d, b = %d\n", a, b); }
};

int main() {
    int a = 42, b = 1024;
    // Function<int(int, int)> f1{SimpleFunc};
    // f1(a, b);

    // auto lambda_func = []() { printf("[LambdaFunc] I am lambda!\n"); };
    // Function<void(void)> f2{lambda_func};
    // f2();

    // Foo foo{};
    // Function<void(int, int)> f3{foo};
    // f3(a, b);
    std::print("Hello, World!\n");
    return 0;
}
