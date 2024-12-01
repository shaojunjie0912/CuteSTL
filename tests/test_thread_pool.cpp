// #include <cutestl/thread_pool.h>
#include <cutestl/mtx_thread_pool.h>

#include <iostream>

using namespace cutestl;

void basic_test() {
    MtxThreadPool pool(4);  // 使用4个线程
    auto future1 = pool.Submit([]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::cout << "Task 1 completed\n";
    });
    auto future2 = pool.Submit([]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Task 2 completed\n";
    });

    future1.get();  // 等待任务1完成
    future2.get();  // 等待任务2完成
    std::cout << "All tasks completed\n";
}

int main() {
    basic_test();
    return 0;
}
