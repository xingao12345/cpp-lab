#include <iostream>
#include <utility>

void f(int& )       { std::cout << "int&        （左值引用）\n"; }
void f(int&& )      { std::cout << "int&&       （右值引用）\n"; }
void f(const int& ) { std::cout << "const int&  （const 左值引用）\n"; }

int main() {
    int a = 10;
    const int b = 20;

    std::cout << "f(a)             : "; f(a);
    std::cout << "f(b)             : "; f(b);
    std::cout << "f(42)            : "; f(42);
    std::cout << "f(a + 1)         : "; f(a + 1);
    std::cout << "f(std::move(a))  : "; f(std::move(a));

    return 0;
}