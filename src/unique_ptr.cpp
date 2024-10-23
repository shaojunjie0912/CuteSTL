#include <iostream>
#include <memory>
#include <utility>
#include <vector>

using std::cout;
using std::endl;

template <typename T, typename U>
T Exchange(T& dst, U&& val) {
    T tmp = std::move(dst);
    dst = std::forward<U>(val);
    return tmp;
}

template <typename T>
struct DefaultDeleter {
    void operator()(T* p) const {
        std::cout << "DefaultDeleter: delete" << std::endl;
        delete p;
    }
};

template <>
struct DefaultDeleter<FILE> {
    void operator()(FILE* p) const { fclose(p); }
};

template <typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr {
public:
    UniquePtr() : ptr_(nullptr) {}

    explicit UniquePtr(T* ptr) : ptr_(ptr) {}

    ~UniquePtr() {
        if (ptr_) {
            Deleter{}(ptr_);
        }
    }

    UniquePtr(UniquePtr const& other) = delete;

    UniquePtr(UniquePtr&& other) noexcept {
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
    }

    UniquePtr& operator=(UniquePtr const& other) = delete;

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            Deleter{}(ptr_);
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }

    T* operator->() const { return ptr_; }

    T& operator*() const { return *ptr_; }

    T* Get() const { return ptr_; }

private:
    T* ptr_ = nullptr;
};

template <typename T, typename... Args>
UniquePtr<T> MakeUnique(Args&&... args) {
    return UniquePtr<T>{new T{std::forward<Args>(args)...}};
}

struct Animal {
    virtual void Speak() = 0;
    virtual ~Animal() {}
};

struct Dog : public Animal {
    void Speak() override { cout << "Dog Speak" << endl; }
};

struct Cat : public Animal {
    void Speak() override { cout << "Cat Speak" << endl; }
};

int main() {
    std::vector<UniquePtr<Animal>> zoo;
    zoo.push_back(MakeUnique<Cat>());
    zoo.push_back(MakeUnique<Dog>());
    for (auto const& a : zoo) {
        a->Speak();
    }
    return 0;
}
