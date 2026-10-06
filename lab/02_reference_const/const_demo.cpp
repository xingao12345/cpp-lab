#include <iostream>

int main() {
    const int a = 10;
    int& r = a;          // ← 第 5 行，问题在这
    r = 20;
    std::cout << a << "\n";
    return 0;
}