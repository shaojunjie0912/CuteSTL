#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

// C++17/20 实现的一个简单、可靠的固定大小线程池
// 特性：
//   * Submit 任意可调用对象，返回 std::future<R>
//   * 析构或 Shutdown() 会阻止新任务并等待工作线程退出
//   * 异常在 future.get() 时重新抛出
namespace cutestl {
class ThreadPool {
public:
    explicit ThreadPool(std::size_t thread_count) : stopping_(false) {
        if (thread_count == 0) {
            throw std::invalid_argument("thread_count must be > 0");
        }
        workers_.reserve(thread_count);
        try {
            for (std::size_t i = 0; i < thread_count; ++i) {
                workers_.emplace_back([this] {
                    this->WorkerLoop();  // 启动工作线程循环
                });
            }
        } catch (...) {
            // 如果部分线程已创建，确保清理。
            Shutdown();
            throw;
        }
    }

    // 禁止拷贝和移动
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;

    ~ThreadPool() { Shutdown(); }

    // 提交任务：接受任意可调用与参数，返回 future<返回类型>。
    template <class F, class... Args>
    auto Submit(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
        using R = std::invoke_result_t<F, Args...>;

        // 把原本需要参数的可调用对象 f 与它的参数 args... 预先“拼”好, 变成一个“无参可调用对象”
        // 并用 packaged_task 包装以拿到 future。
        // 用 lambda + apply 避免 std::bind 的语义惊喜，完美转发并保持移动
        auto packer = [fn = std::forward<F>(f),
                       tup = std::make_tuple(std::forward<Args>(args)...)]() mutable -> R {
            return std::apply(std::move(fn), std::move(tup));
        };

        // std::promise + std::future：手动设置结果
        // std::packaged_task + std::future：任务执行后自动产生结果
        // 执行 packaged_task 相当于执行它内部的函数，并自动将结果放入 future 对象中

        // HACK: packaged_task 只能移动, 不能拷贝, 所以需要智能指针包装
        // 不然 lambda 里面只能引用捕获, 但是 packaged_task 栈上变量离开作用域自动销毁, 造成悬垂引用
        // 此外, 任务队列是 std::function, 它具有值语义, 要求所包装的可调用对象必须是可拷贝构造的
        auto task = std::make_shared<std::packaged_task<R()>>(std::move(packer));
        std::future<R> fut = task->get_future();  // 获取 future (因为 task 可能在其他线程被调用)

        {
            std::lock_guard lk{mtx_};
            if (stopping_) {
                throw std::runtime_error("ThreadPool is stopping; cannot submit.");
            }
            // NOTE: 将 task 包装为无参<void()>任务，并添加到任务队列
            // 在工作线程中执行任务。异常由 packaged_task 捕获并传递给 future。
            tasks_.emplace([task]() noexcept { (*task)(); });
        }
        cv_.notify_one();
        return fut;
    }

    // 显式关停：阻止新任务、等待队列清空并回收线程。
    void Shutdown() noexcept {
        {
            std::lock_guard lk{mtx_};
            if (stopping_) {  // 如果已经停止，直接返回
                return;
            }
            stopping_ = true;  // 设置停止标志
        }
        cv_.notify_all();
        workers_.clear();  // 清空工作线程 (jthread 会自动 join)
    }

    std::size_t Size() const noexcept { return workers_.size(); }

private:
    // 工作线程循环
    void WorkerLoop() {
        while (true) {
            std::function<void()> job;  // 任务
            {
                std::unique_lock lk{mtx_};
                // 等到有任务，或线程池进入停止状态。
                cv_.wait(lk, [this] { return stopping_ || !tasks_.empty(); });
                if (stopping_ && tasks_.empty()) {  // 停止且队列空，安全退出
                    return;                         // NOTE: 工作线程会消费完所有任务, 然后退出
                }
                job = std::move(tasks_.front());  // 从队列中取出任务
                tasks_.pop();
            }
            // NOTE: 在锁外执行任务，避免阻塞生产者或其他工作线程。
            job();
        }
    }

    mutable std::mutex mtx_;                   // 互斥锁
    std::condition_variable cv_;               // 条件变量
    std::vector<std::jthread> workers_;        // 工作线程 (C++20 jthread)
    std::queue<std::function<void()>> tasks_;  // 任务队列
    bool stopping_;                            // 停止标志
};
}  // namespace cutestl
