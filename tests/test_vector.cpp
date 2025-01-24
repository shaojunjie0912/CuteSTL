#include <fmt/core.h>

#include <cutestl/vector.hpp>
#include <string>
#include <vector>

using namespace cutestl;

struct A {
    std::string name_;
    int age_;
};

int main() {
    using CuteAVec = cutestl::Vector<A>;
    using StdAVec = std::vector<A>;
    StdAVec std_a_vec{{"sjj", 23}, {"drr", 21}};
    std_a_vec.emplace(std_a_vec.end(), "balala", 3);
    std_a_vec.emplace(std_a_vec.begin() + 1, "notme", 4);
    for (auto const& _ : std_a_vec) {
        fmt::println("name = {}, age = {}\n", _.name_, _.age_);
    }
    fmt::println("===========================");
    CuteAVec cute_a_vec{{"sjj", 23}, {"drr", 21}};
    cute_a_vec.Emplace(cute_a_vec.end(), "balala", 3);
    cute_a_vec.Emplace(cute_a_vec.begin() + 1, "notme", 4);
    cute_a_vec.EmplaceBack("lastlast", 111);
    for (auto const& _ : cute_a_vec) {
        fmt::println("name = {}, age = {}\n", _.name_, _.age_);
    }

    return 0;
}