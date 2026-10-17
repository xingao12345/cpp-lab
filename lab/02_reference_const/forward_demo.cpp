#include <iostream>
#include <type_traits>
#include <utility>

// ═══════════════════════════════════════════════════════════
// 被调用的目标函数：两个重载，靠【值类别】区分
// ═══════════════════════════════════════════════════════════
void target(int&)  { std::cout << "    → target(int&)   左值版本\n"; }
void target(int&&) { std::cout << "    → target(int&&)  右值版本\n"; }

// ═══════════════════════════════════════════════════════════
// 实验 2：不转发
// ═══════════════════════════════════════════════════════════
template <typename T>
void no_forward(T&& x) {
    std::cout << "  [推导] T = "
              << (std::is_lvalue_reference<T>::value ? "int&" : "int") << "\n";
    target(x);            // x 有名字 → 永远是左值，右值性丢失
}

// ═══════════════════════════════════════════════════════════
// 实验 3：完美转发
// ═══════════════════════════════════════════════════════════
template <typename T>
void with_forward(T&& x) {
    std::cout << "  [推导] T = "
              << (std::is_lvalue_reference<T>::value ? "int&" : "int") << "\n";
    target(std::forward<T>(x));     // 保持 x 原来的值类别
}

// ═══════════════════════════════════════════════════════════
// 实验 4：移动构造里漏写 std::move 的后果
// ═══════════════════════════════════════════════════════════
struct Inner {
    Inner() = default;
    Inner(const Inner&)     { std::cout << "    Inner 拷贝构造\n"; }
    Inner(Inner&&) noexcept { std::cout << "    Inner 移动构造\n"; }
};

struct OuterBad {
    Inner inner_;
    OuterBad() = default;
    // ❌ 漏了 std::move：o.inner_ 是【具名成员】→ 左值 → 走拷贝构造
    OuterBad(OuterBad&& o) noexcept : inner_(o.inner_) {}
};

struct OuterGood {
    Inner inner_;
    OuterGood() = default;
    // ✅ 用 std::move 把它重新标记成右值
    OuterGood(OuterGood&& o) noexcept : inner_(std::move(o.inner_)) {}
};

// ═══════════════════════════════════════════════════════════
int main() {
    std::cout << "══════ 实验 1：具名的右值引用，本身是左值 ══════\n";
    int&& r = 42;
    int& x = r;                                  // 这一行为什么能编译？
    std::cout << "  int&& r = 42;\n";
    std::cout << "  int&  x = r;     ← 编译通过！x = " << x << "\n";
    std::cout << "  结论：r 的【类型】是 int&&，但 r【有名字】→ r 是左值\n\n";

    std::cout << "══════ 实验 2：不转发（右值性丢失）══════\n";
    int a = 10;
    std::cout << "no_forward(a)   传左值 a:\n";
    no_forward(a);
    std::cout << "no_forward(42)  传右值 42:\n";
    no_forward(42);
    std::cout << "  ↑ 明明传进去的是右值，target 收到的却是左值\n\n";

    std::cout << "══════ 实验 3：完美转发（保留值类别）══════\n";
    std::cout << "with_forward(a)   传左值 a:\n";
    with_forward(a);
    std::cout << "with_forward(42)  传右值 42:\n";
    with_forward(42);
    std::cout << "  ↑ 值类别被完整保留 ✅\n\n";

    std::cout << "══════ 实验 4：移动构造里漏写 std::move ══════\n";
    std::cout << "OuterBad  —— 写成 inner_(o.inner_):\n";
    OuterBad a1;
    OuterBad b1 = std::move(a1);
    std::cout << "  ↑ 期望走【移动】，实际走了【拷贝】，性能白丢\n\n";

    std::cout << "OuterGood —— 写成 inner_(std::move(o.inner_)):\n";
    OuterGood a2;
    OuterGood b2 = std::move(a2);
    std::cout << "  ↑ 正确走移动构造 ✅\n";

    (void)b1;
    (void)b2;
    return 0;
}
