#include <memory>
#include <utility>

namespace cutestl {

// 控制块通常与被管理对象一起分配, 此处简化实现, 独立管理
struct ControlBlock {
    int ref_cnt_;  // 引用计数
    ControlBlock() : ref_cnt_(1) {}
};

template <typename T>
class SimpleSharedPtr {
public:
    // 默认构造函数(无参)
    SimpleSharedPtr() : p_(nullptr), ctrl_(nullptr) {}

    explicit SimpleSharedPtr(T* p) : p_(p) { ctrl_ = p ? new ControlBlock{} : nullptr; }

    ~SimpleSharedPtr() { Release(); }

    SimpleSharedPtr(SimpleSharedPtr const& other) : p_(other.p_), ctrl_(other.ctrl_) {
        if (ctrl_) {
            ++ctrl_->ref_cnt_;
        }
    }

    SimpleSharedPtr& operator=(SimpleSharedPtr const& other) {
        if (this != &other) {
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
    SimpleSharedPtr(SimpleSharedPtr&& other)
        : p_(std::exchange(other.p_, nullptr)), ctrl_(std::exchange(other.ctrl_, nullptr)) {}

    SimpleSharedPtr& operator=(SimpleSharedPtr&& other) {
        if (this != &other) {
            Release();
            p_ = std::exchange(other.p_, nullptr);
            ctrl_ = std::exchange(other.ctrl_, nullptr);
        }
        return *this;
    }

    T& operator*() const { return *p_; }

    T* operator->() const { return p_; }

public:
    int UseCount() const { return ctrl_ ? ctrl_->ref_cnt_ : 0; }

    T* Get() const { return p_; }

    void Reset(T* new_p = nullptr) {
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
    T* p_;                // 指向被管理对象的指针
    ControlBlock* ctrl_;  // 指向控制块的指针
};

}  // namespace cutestl
