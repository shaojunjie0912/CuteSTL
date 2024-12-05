// 普通队列

#include <deque>

namespace cutestl {

template <typename T, typename Container = std::deque<T>>  // TODO: cutestl::deque
class Queue {
public:
    using value_type = typename Container::value_type;
    using size_type = typename Container::size_type;
};

}  // namespace cutestl
