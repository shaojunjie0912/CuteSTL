#include <cutestl/queue.h>

#include <functional>
#include <iostream>

using namespace std;
using namespace cutestl;

class Foo {
public:
    Foo() = default;
    Foo(const Foo&) = delete;
    Foo(Foo&&) = default;
    int a;
};

int main() {
    using FuncType = std::function<void()>;
    MtxQueue<FuncType> func_queue;
    std::function<void()> func1{[] { cout << "ok"; }};
    MtxQueue<Foo> foo_queue;
    Foo foo1{};
    Foo foo2{};
    // foo_queue.Push(foo1);
    func_queue.Push(func1);
}
