#include <fmt/core.h>

#include <cutestl/vector.hpp>
#include <string>
#include <vector>

using namespace cutestl;

int main() {
    using StrVec = cutestl::Vector<std::string>;
    // std::vector std_vec(4, "std");
    StrVec vec2(3, "ds");
    vec2.Print();
    StrVec vec3{vec2.begin(), vec2.end()};
    vec3.Print();
    return 0;
}