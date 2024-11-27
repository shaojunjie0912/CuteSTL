#include <cutestl/mtx_queue_c++11.h>

#include <functional>
#include <future>
#include <memory>
#include <thread>
#include <type_traits>

class ThreadPool {
public:
    template <typename F, typename... Args>
    auto Submit(F &&f, Args &&...args) -> std::future<decltype(f(args...))>  // 1
    {
        std::function<decltype(f(args...))()> func =
            std::bind(std::forward<F>(f), std::forward<Args>(args)...);                      // 2
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);  // 3

        auto task_ptr1 = std::make_shared<std::packaged_task<decltype(f(args...))()>>(
            [&] { std::forward<F>(f)(std::forward<Args>(args)...); });  // 3

        std::function<void()> wrapper_func = [task_ptr] { (*task_ptr)(); };  // 4

        // 队列通用安全封包函数，并压入安全队列
        m_queue.enqueue(wrapper_func);

        // 唤醒一个等待中的线程
        m_conditional_lock.notify_one();  // 5

        // 返回先前注册的任务指针
        return task_ptr->get_future();
    }

private:
};

int main() {}
