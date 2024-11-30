#pragma once

#include <cutestl/mtx_queue.h>

#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <thread>
#include <type_traits>

class ThreadPool {
public:
    explicit ThreadPool(std::size_t num_threads = std::thread::hardware_concurrency())
        : stop_(false) {
        for (std::size_t i{0}; i < num_threads; ++i) {
            thread_pool_.emplace_back([this] {
                while (!stop_.load()) {
                    auto task = task_queue_.Pop();
                    task();
                }
            });
        }
    }

    ~ThreadPool() {
        stop_.store(true);
        for (auto &t : thread_pool_) {
            t.join();
        }
    }

public:
    ThreadPool(ThreadPool const &) = delete;
    ThreadPool(ThreadPool &&) = delete;

public:
    template <typename F, typename... Args>
    auto Submit(F &&f, Args &&...args) {
        using RT = std::invoke_result_t<F, Args...>;
        auto task = std::make_shared<std::packaged_task<RT(Args...)>>(
            [&] { std::forward<F>(f)(std::forward<Args>(args)...); });
        task_queue_.Push([&task] { (*task)(); });
        return task->get_future();
    }

private:
    MtxQueue<std::function<void()>> task_queue_;  // 任务队列
    std::vector<std::thread> thread_pool_;        // 线程池
    std::atomic<bool> stop_;                      // 线程池停止标志
};
