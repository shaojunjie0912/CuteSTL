#include <algorithm>
#include <cstddef>
#include <iterator>
#include <stdexcept>

namespace cutestl {

template <typename T, size_t N>
class Array {
private:
    T elements_[N];

public:
    explicit Array() {}
};

}  // namespace cutestl

using namespace cutestl;

int main() {}
