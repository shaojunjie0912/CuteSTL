// 智能指针控制块
template <typename T>
struct SpControlBlock {
    // NOTE: 使用 new 创造的 SpControlBlock 的 refcnt 为 1
    explicit SpControlBlock(T* ptr) : data_(ptr), refcnt_(1) {}
    SpControlBlock(SpControlBlock&&) = delete;
    T* data_;
    int refcnt_;
};

template <typename T>
class SharedPtr {
public:
    explicit SharedPtr(T* ptr) : cb_(new SpControlBlock<T>(ptr)) {}
    SharedPtr(SharedPtr const& other) { ++cb_->refcnt_; }
    ~SharedPtr() {
        --cb_->refcnt_;
        if (cb_->refcnt_ == 0) {
            delete cb_;
        }
    }

private:
    SpControlBlock<T>* cb_;
};

struct Student {
    char const* name_;
    int age_;
    explicit Student(char const* name, int age) : name_(name), age_(age) {}
};

int main() {
    SharedPtr p0{new Student("ShaoJunjie", 23)};
    return 0;
}
