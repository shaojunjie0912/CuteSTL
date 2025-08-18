#include <cutestl/list.hpp>
#include <iostream>

using namespace std;
using namespace cutestl;

int main() {
    List<int> l;
    for (int i = 1; i <= 3; ++i) {
        l.Insert(l.Begin(), i);
    }
    l.Insert(++l.Begin(), 444);
    auto it = l.Insert(--l.End(), 3, 1024);
    l.Show();
    std::cout << *(--it) << std::endl;
}