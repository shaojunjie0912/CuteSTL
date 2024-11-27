#include <cutestl/mtx_queue.h>

// #if __cplusplus >= 202002L
// #include <concepts>
// #endif
#include <functional>
#include <future>
#include <memory>
#include <thread>
#include <type_traits>

template <typename F, typename... Args>
std::future<std::invoke_result_t<F, Args...>> Submit(F &&f, Args &&...args) {
    auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))(Args...)>>(
        [&] { f(std::forward<Args>(args)...); });
}

class ThreadPool {
private:
};

int main() {}
