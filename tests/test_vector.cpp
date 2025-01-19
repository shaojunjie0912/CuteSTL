#include <fmt/core.h>

#include <cutestl/vector.hpp>
#include <string>
#include <vector>

using namespace cutestl;

int main() {
    using StrVec = cutestl::Vector<std::string>;
    std::vector<std::string> std_vec(4, "std");
    StrVec vec1(3, "0");
    StrVec vec2 = {"1", "2"};
    vec2 = {"5", "6", "d"};
    vec2.Print();
    // vec1.Reserve(4);
    // vec1.Insert(vec1.begin(), 2, "c");
    // vec1.Print();
    // vec1.Erase(vec1.begin(), vec1.begin() + 3);
    // vec1.Print();
    // vec1.PushBack("dasdsa");
    // vec1.PushBack("dd");
    // vec1.Print();
    // vec1.PopBack();
    // vec1.Print();
    return 0;
}