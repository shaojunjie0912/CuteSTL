#include <fmt/core.h>

#include <cutestl/vector.hpp>

using namespace cutestl;

int main() {
    // Vector<int> v1{1, 2, 3};
    Vector<int> v1(3);
    for (std::size_t i{0}; i < v1.Size(); ++i) {
        v1[i] = i;
        // cout << v1[i] << " " << "\n";
        fmt::print("{}\n", v1[i]);
    }
    // cout << *v1.data_ << "\n";
    // cout << *(v1.data_ + sizeof(int)) << "\n";
    // cout << *(v1.data_ + 2 * sizeof(int)) << "\n";
}