#pragma once

#include <algorithm>
#include <atomic>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

//
#include <cutestl/unique_ptr.hpp>

// TODO: 先不加空间配置器

namespace cutestl {

struct ControlBlock {
    std::atomic<long> ref_cnt_;

    ControlBlock() noexcept : ref_cnt_(1) {}

    ControlBlock(ControlBlock &&) = delete;

    void IncRef() noexcept { ref_cnt_.fetch_add(1, std::memory_order_relaxed); }

    void DecRef() noexcept {
        if (ref_cnt_.fetch_sub(1, std::memory_order_relaxed) == 1) {
            delete this;
        }
    }

    long CntRef() const noexcept { return ref_cnt_.load(std::memory_order_relaxed); }

    virtual ~ControlBlock() = default;
};

template <class Tp, class Deleter>
struct ControlBlockImpl final : ControlBlock {
    Tp *p_;
    [[no_unique_address]] Deleter deleter_;

    explicit ControlBlockImpl(Tp *ptr) noexcept : p_(ptr) {}

    explicit ControlBlockImpl(Tp *ptr, Deleter _Deleter) noexcept
        : p_(ptr), deleter_(std::move(_Deleter)) {}

    ~ControlBlockImpl() noexcept override { deleter_(p_); }
};

template <class Tp>
struct SharedPtr {
private:
    Tp *p_;
    ControlBlock *ctrl_;

    // template <class>
    // friend struct SharedPtr;

    explicit SharedPtr(Tp *p, ControlBlock *ctrl) noexcept : p_(p), ctrl_(ctrl) {}

public:
    using element_type = Tp;
    using pointer = Tp *;

    SharedPtr(std::nullptr_t = nullptr) noexcept : ctrl_(nullptr) {}

    template <class Yp, std::enable_if_t<std::is_convertible_v<Yp *, Tp *>, int> = 0>
    explicit SharedPtr(Yp *ptr)
        : p_(ptr), ctrl_(new ControlBlockImpl<Yp, DefaultDeleter<Yp>>(ptr)) {
        _S_setupEnableSharedFromThis(p_, ctrl_);
    }

    template <class Yp, class Deleter, std::enable_if_t<std::is_convertible_v<Yp *, Tp *>, int> = 0>
    explicit SharedPtr(Yp *ptr, Deleter _Deleter)
        : p_(ptr), ctrl_(new ControlBlockImpl<Yp, Deleter>(ptr, std::move(_Deleter))) {
        _S_setupEnableSharedFromThis(p_, ctrl_);
    }

    template <class Yp, class Deleter, std::enable_if_t<std::is_convertible_v<Yp *, Tp *>, int> = 0>
    explicit SharedPtr(UniquePtr<Yp, Deleter> &&ptr) : SharedPtr(ptr.release(), ptr.getDeleter()) {}

    template <class Yp>
    inline friend SharedPtr<Yp> _S_makeSharedFused(Yp *ptr, ControlBlock *owner) noexcept;

    SharedPtr(SharedPtr const &other) noexcept : p_(other.p_), ctrl_(other.ctrl_) {
        if (ctrl_) {
            ctrl_->IncRef();
        }
    }

    template <class Yp, std::enable_if_t<std::is_convertible_v<Yp *, Tp *>, int> = 0>
    SharedPtr(SharedPtr<Yp> const &other) noexcept : p_(other.p_), ctrl_(other.ctrl_) {
        if (ctrl_) {
            ctrl_->IncRef();
        }
    }

    SharedPtr(SharedPtr &&other) noexcept : p_(other.p_), ctrl_(other.ctrl_) {
        other.p_ = nullptr;
        other.ctrl_ = nullptr;
    }

    template <class Yp, std::enable_if_t<std::is_convertible_v<Yp *, Tp *>, int> = 0>
    SharedPtr(SharedPtr<Yp> &&other) noexcept : p_(other.p_), ctrl_(other.ctrl_) {
        other.p_ = nullptr;
        other.ctrl_ = nullptr;
    }

    template <class Yp>
    SharedPtr(SharedPtr<Yp> const &other, Tp *ptr) noexcept : p_(ptr), ctrl_(other.ctrl_) {
        if (ctrl_) {
            ctrl_->IncRef();
        }
    }

    template <class Yp>
    SharedPtr(SharedPtr<Yp> &&other, Tp *ptr) noexcept : p_(ptr), ctrl_(other.ctrl_) {
        other.p_ = nullptr;
        other.ctrl_ = nullptr;
    }

    SharedPtr &operator=(SharedPtr const &other) noexcept {
        if (this == &other) {
            return *this;
        }
        if (ctrl_) {
            ctrl_->DecRef();
        }
        p_ = other.p_;
        ctrl_ = other.ctrl_;
        if (ctrl_) {
            ctrl_->IncRef();
        }
        return *this;
    }

    SharedPtr &operator=(SharedPtr &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        if (ctrl_) {
            ctrl_->DecRef();
        }
        p_ = other.p_;
        ctrl_ = other.ctrl_;
        other.p_ = nullptr;
        other.ctrl_ = nullptr;
        return *this;
    }

    template <class Yp, std::enable_if_t<std::is_convertible_v<Yp *, Tp *>, int> = 0>
    SharedPtr &operator=(SharedPtr<Yp> const &other) noexcept {
        if (this == &other) {
            return *this;
        }
        if (ctrl_) {
            ctrl_->DecRef();
        }
        p_ = other.p_;
        ctrl_ = other.ctrl_;
        if (ctrl_) {
            ctrl_->IncRef();
        }
        return *this;
    }

    template <class Yp, std::enable_if_t<std::is_convertible_v<Yp *, Tp *>, int> = 0>
    SharedPtr &operator=(SharedPtr<Yp> &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        if (ctrl_) {
            ctrl_->DecRef();
        }
        p_ = other.p_;
        ctrl_ = other.ctrl_;
        other.p_ = nullptr;
        other.ctrl_ = nullptr;
        return *this;
    }

    void reset() noexcept {
        if (ctrl_) {
            ctrl_->DecRef();
        }
        ctrl_ = nullptr;
        p_ = nullptr;
    }

    template <class Yp>
    void reset(Yp *ptr) {
        if (ctrl_) {
            ctrl_->DecRef();
        }
        p_ = nullptr;
        ctrl_ = nullptr;
        p_ = ptr;
        ctrl_ = new ControlBlockImpl<Yp, DefaultDeleter<Yp>>(ptr);
        _S_setupEnableSharedFromThis(p_, ctrl_);
    }

    template <class Yp, class Deleter>
    void reset(Yp *ptr, Deleter _Deleter) {
        if (ctrl_) {
            ctrl_->DecRef();
        }
        p_ = nullptr;
        ctrl_ = nullptr;
        p_ = ptr;
        ctrl_ = new ControlBlockImpl<Yp, Deleter>(ptr, std::move(_Deleter));
        _S_setupEnableSharedFromThis(p_, ctrl_);
    }

    ~SharedPtr() noexcept {
        if (ctrl_) {
            ctrl_->DecRef();
        }
    }

    long use_count() noexcept { return ctrl_ ? ctrl_->CntRef() : 0; }

    bool unique() noexcept { return ctrl_ ? ctrl_->CntRef() == 1 : true; }

    template <class Yp>
    bool operator==(SharedPtr<Yp> const &other) const noexcept {
        return p_ == other.p_;
    }

    template <class Yp>
    bool operator!=(SharedPtr<Yp> const &other) const noexcept {
        return p_ != other.p_;
    }

    template <class Yp>
    bool operator<(SharedPtr<Yp> const &other) const noexcept {
        return p_ < other.p_;
    }

    template <class Yp>
    bool operator<=(SharedPtr<Yp> const &other) const noexcept {
        return p_ <= other.p_;
    }

    template <class Yp>
    bool operator>(SharedPtr<Yp> const &other) const noexcept {
        return p_ > other.p_;
    }

    template <class Yp>
    bool operator>=(SharedPtr<Yp> const &other) const noexcept {
        return p_ >= other.p_;
    }

    template <class Yp>
    bool owner_before(SharedPtr<Yp> const &other) const noexcept {
        return ctrl_ < other.ctrl_;
    }

    template <class Yp>
    bool owner_equal(SharedPtr<Yp> const &other) const noexcept {
        return ctrl_ == other.ctrl_;
    }

    void swap(SharedPtr &other) noexcept {
        std::swap(p_, other.p_);
        std::swap(ctrl_, other.ctrl_);
    }

    Tp *get() const noexcept { return p_; }

    Tp *operator->() const noexcept { return p_; }

    std::add_lvalue_reference_t<Tp> operator*() const noexcept { return *p_; }

    explicit operator bool() const noexcept { return p_ != nullptr; }
};

}  // namespace cutestl
