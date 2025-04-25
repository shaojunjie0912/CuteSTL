#include <fmt/core.h>

#include <cutestl/thread_safe_shared_ptr.hpp>
#include <iostream>
#include <thread>
#include <vector>

using namespace cutestl;

struct Foo {
    int a{42};
};

int main() {
    ThreadSafeSharedPtr<Foo> sptr{new Foo};
    fmt::println("开始, 引用计数 = {}", sptr.UseCount());
    int const num_threads{2};
    {
        std::vector<std::jthread> threads_copy;
        for (int i{0}; i < num_threads; ++i) {
            threads_copy.emplace_back([&] {
                static int id = 0;
                auto local_sptr{sptr};
                // fmt::println("线程 {} 拷贝了 shared_ptr, 引用计数 = {}", id++,
                //              local_sptr.UseCount());
            });
        }
    }
    fmt::println("结束, 引用计数 = {}", sptr.UseCount());
}
