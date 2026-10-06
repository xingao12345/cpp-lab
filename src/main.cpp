#include <iostream>
#include <vector>

int main() {//main函数
    std::vector<int> v{1, 2, 3};
    for (int x : v) std::cout << x << ' ';
    std::cout << "\nC++ standard: " << __cplusplus << '\n';
    return 0;
}
