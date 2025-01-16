// 基于互斥锁+条件变量的单任务队列线程池

#pragma once

#include <condition_variable>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>

#include "functional.h"

namespace cutestl {

class MtxThreadPool {
public:
    explicit MtxThreadPool(std::size_t num_threads = std::thread::hardware_concurrency())
        : stop_(false) {
        for (std::size_t i{0}; i < num_threads; ++i) {
            thread_pool_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    // {} 防止 task() 耗时使得锁无法释放
                    {
                        std::unique_lock lk{mtx_};
                        // 条件顺序
                        cv_.wait(lk, [this] { return stop_ || !task_queue_.empty(); });
                        // 条件组合
                        if (stop_ && task_queue_.empty()) {
                            return;
                        }
                        task = task_queue_.front();
                        task_queue_.pop();
                    }
                    task();
                }
            });
        }
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    ~MtxThreadPool() {
        {
            std::unique_lock lk{mtx_};
            stop_ = true;
        }
        cv_.notify_all();
        for (auto &t : thread_pool_) {
            if (t.joinable()) {
                t.join();
            }
        }
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

public:
    MtxThreadPool(MtxThreadPool const &) = delete;
    MtxThreadPool(MtxThreadPool &&) = delete;

public:
    template <typename F, typename... Args>
    auto Submit(F &&f, Args &&...args) {
        using RT = std::invoke_result_t<F, Args...>;
        // 智能指针管理 task 生命周期
        auto task = std::make_shared<std::packaged_task<RT(Args...)>>(
            [&] { std::forward<F>(f)(std::forward<Args>(args)...); });
        std::future<RT> res = task->get_future();
        {
            std::unique_lock lk{mtx_};
            task_queue_.emplace([task = std::move(task)] { (*task)(); });
        }
        cv_.notify_one();
        return res;
    }

private:
    std::queue<std::function<void()>> task_queue_;  // 任务队列
    std::vector<std::thread> thread_pool_;          // 线程池
    bool stop_;                                     // 线程池停止标志
    std::mutex mtx_;                                // 互斥体
    std::condition_variable cv_;                    // 条件变量
};

}  // namespace cutestl
