#include <fmt/core.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <utility>

namespace cutestl {

// 控制块通常与被管理对象一起分配, 此处简化实现, 独立管理
struct ControlBlock {
    std::atomic<int> ref_cnt_;  // 引用计数
    ControlBlock() : ref_cnt_(1) {}
};

template <typename T>
class ThreadSafeSharedPtr {
public:
    // 默认构造函数(无参)
    ThreadSafeSharedPtr() : p_(nullptr), ctrl_(nullptr) {}

    explicit ThreadSafeSharedPtr(T* p) : p_(p) { ctrl_ = p ? new ControlBlock{} : nullptr; }

    ~ThreadSafeSharedPtr() { Release(); }

    ThreadSafeSharedPtr(ThreadSafeSharedPtr const& other) {
        std::lock_guard lk{other.mtx_};  // NOTE: 用 other.mtx_ 因为新对象尚未构造完成
        p_ = other.p_;
        ctrl_ = other.ctrl_;
        if (ctrl_) {
            ++ctrl_->ref_cnt_;
        }
    }

    ThreadSafeSharedPtr& operator=(ThreadSafeSharedPtr const& other) {
        if (this != &other) {
            std::scoped_lock lk{mtx_, other.mtx_};  // NOTE: 避免死锁, 同时锁定两个互斥体
            Release();
            p_ = other.p_;
            ctrl_ = other.ctrl_;
            if (ctrl_) {
                ++ctrl_->ref_cnt_;
            }
        }
        return *this;
    }

    // NOTE: 资源移动, 不增加不减少引用计数
    ThreadSafeSharedPtr(ThreadSafeSharedPtr&& other) {
        std::lock_guard lk{other.mtx_};
        p_ = std::exchange(other.p_, nullptr);
        ctrl_ = std::exchange(other.ctrl_, nullptr);
    }

    ThreadSafeSharedPtr& operator=(ThreadSafeSharedPtr&& other) {
        if (this != &other) {
            std::scoped_lock lk{mtx_, other.mtx_};
            Release();
            p_ = std::exchange(other.p_, nullptr);
            ctrl_ = std::exchange(other.ctrl_, nullptr);
        }
        return *this;
    }

    T& operator*() const {
        std::lock_guard lk{mtx_};
        return *p_;
    }

    T* operator->() const {
        std::lock_guard lk{mtx_};
        return p_;
    }

public:
    int UseCount() const {
        std::lock_guard lk{mtx_};
        return ctrl_ ? ctrl_->ref_cnt_.load() : 0;
    }

    T* Get() const { return p_; }

    void Reset(T* new_p = nullptr) {
        std::lock_guard lk{mtx_};
        Release();
        p_ = new_p;
        ctrl_ = p_ ? new ControlBlock{} : nullptr;
    }

private:
    void Release() {
        if (ctrl_) {
            // 引用计数 - 1, 如果为 0, 则释放资源
            if (--ctrl_->ref_cnt_ == 0) {
                delete p_;
                delete ctrl_;
            }
        }
        p_ = nullptr;
        ctrl_ = nullptr;
    }

private:
    T* p_;                    // 指向被管理对象的指针
    ControlBlock* ctrl_;      // 指向控制块的指针
    mutable std::mutex mtx_;  // 互斥体 NOTE: mutable: 用于 const 成员函数
};

}  // namespace cutestl
