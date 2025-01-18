#include <fmt/core.h>

#include <cutestl/vector.hpp>
#include <vector>

using namespace cutestl;

struct A {
    int c;
};

// void Test(std::vector<int> const& v) { auto x{std::move(x)}; }

int main() {
    std::vector<std::vector<int>> std_vec{{3, 4}, {2, 3}};
    // Vector<int> cute_vec1(4, 3);
    // Vector cute_vec2{1, 1, 2, 3};
    // Vector cute_vec3{cute_vec2};
    // Vector cute_vec4 = std::move(cute_vec2);
    // // cute_vec4.Print();
    // // cute_vec2.Print();
    Vector<Vector<int>> cute_vec5{{1, 2, 3}, {4, 5, 6}};

    return 0;
}