#include <iostream>

int main() {
    int x = 1;
    int y = 2;

    const int* p1 = &x;
    int* const p2 = &x;
    const int* const p3 = &x;

    *p1 = 10;      // ①
    p1 = &y;       // ②
    *p2 = 10;      // ③
    p2 = &y;       // ④
    *p3 = 10;      // ⑤
    p3 = &y;       // ⑥

    return 0;
}