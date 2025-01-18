#include <fmt/core.h>

#include <cutestl/vector.hpp>
#include <string>
#include <vector>

using namespace cutestl;

int main() {
    using StrVec = cutestl::Vector<std::string>;
    // std::vector std_vec(4, "std");
    StrVec vec1(2, "str");
    vec1.Print();
    StrVec vec2(3, "aaa");
    vec1 = vec2;
    vec1.Print();
    return 0;
}