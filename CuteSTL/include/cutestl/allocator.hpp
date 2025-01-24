#pragma once

#include <cstddef>
#include <memory>

namespace cutestl {

template <typename T>
class Allocator {
public:
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T*;

public:
    static pointer Allocate(size_type n) { return static_cast<pointer>(operator new(n * sizeof(value_type))); }
    static void Deallocate(pointer p) { operator delete(p); }
};

}  // namespace cutestl