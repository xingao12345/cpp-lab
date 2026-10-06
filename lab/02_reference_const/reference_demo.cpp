#include <iostream>

int main() {
    int a = 10;
    int& r = a;              // r 是 a 的别名
    int* p = &a;             // p 是一个变量，里面存着 a 的地址

    std::cout << "a 的值:   " << a << "\n";
    std::cout << "r 的值:   " << r << "\n\n";

    std::cout << "&a = " << &a << "\n";
    std::cout << "&r = " << &r << "   ← 和 &a 完全相同\n";
    std::cout << " p = " << p  << "   ← 也等于 &a\n";
    std::cout << "&p = " << &p << "   ← 和 &a 不同！p 自己是个变量\n\n";

    std::cout << "sizeof(a) = " << sizeof(a) << "\n";
    std::cout << "sizeof(r) = " << sizeof(r) << "   ← 等于 sizeof(a)，引用不额外占内存\n";
    std::cout << "sizeof(p) = " << sizeof(p) << "   ← 指针占 8 字节\n\n";

    r = 99;
    std::cout << "执行 r = 99 之后，a = " << a << "   ← a 被改了\n";

    return 0;
}