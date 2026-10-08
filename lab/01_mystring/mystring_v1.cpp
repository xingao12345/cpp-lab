#include <cstring>
#include <iostream>
#include <utility> // std::swap

class MyString
{
public:
    // ── 构造函数：从 C 字符串构造 ──
    MyString(const char *s = "")
    {
        size_ = std::strlen(s);
        data_ = new char[size_ + 1];
        std::strcpy(data_, s);
        std::cout << "[ctor] " << data_ << "\n";
    }

    // ── 拷贝构造：深拷贝，保证两个对象各自持有独立内存 ──
    // 拷贝构造：深拷贝；对方若为"被移动后的空对象"，也安全
    MyString(const MyString& other) {
        size_ = other.size_;
        data_ = new char[size_ + 1];
        std::strcpy(data_, other.data_ ? other.data_ : "");
        std::cout << "[copy-ctor] " << data_ << "\n";
    }
    // 移动构造：直接接管对方的资源，O(1)
    MyString(MyString&& other) noexcept:data_(other.data_),size_(other.size_)
    {
        other.data_=nullptr;
        other.size_=0;
        std::cout << "[move-ctor] " << (data_ ? data_ : "(null)") << "\n";
    }
    // ── 交换：只交换指针和长度，不碰堆内存，因此不会抛异常 ──
    void swap(MyString &other) noexcept
    {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
    }

    // ── 拷贝赋值运算符：copy-and-swap 实现 ──
    //
    // 【为什么参数是按值传递，而不是 const MyString&？】
    //   按值传递时，实参会先被拷贝构造出一份副本 —— 这一步就是"深拷贝"。
    //   如果深拷贝失败（内存不足抛 std::bad_alloc），异常在进入函数体之前就抛出了，
    //   *this 根本没被碰过，对象保持完好。这就是「异常安全」。
    //
    // 【为什么不需要写 if (this == &other) 自赋值检查？】
    //   执行 b = b 时，同样是先拷贝出一份副本。交换之后，副本持有的是 b 原来的内存，
    //   函数结束时副本析构，释放的是那块旧内存；b 本身始终指向有效内存。
    //   所以不会出现"先释放自己，再从已释放的内存里拷贝数据"的悬垂指针问题。
    //
    // 【对比：朴素的实现（先 delete 再 new）有什么问题？】
    //   delete[] data_;                 // 旧内存已经释放
    //   data_ = new char[size_ + 1];    // ← 如果这里抛 bad_alloc
    //   此时 data_ 里存的还是那个已释放的地址，对象变成"残缺状态"，
    //   之后析构时会 delete[] 一个野指针 → double free。
    //   修法有两种：① 调换顺序（先申请成功、再释放旧的）；
    //              ② 就是这里的 copy-and-swap，天然异常安全且代码复用。
    MyString &operator=(MyString other)
    {
        swap(other);
        return *this;
    } // 函数返回时 other（副本）析构，顺便释放掉 *this 原来的内存

    // ── 析构函数 ──
    ~MyString()
    {
        std::cout << "[dtor] " << (data_ ? data_ : "(null)") << "\n";
        delete[] data_;
    }

    const char *c_str() const { return data_ ? data_ : ""; }
    std::size_t size() const { return size_; }

private:
    char *data_ = nullptr;
    std::size_t size_ = 0;
};

int main() {
    std::cout << "===== 测试 1：移动构造 =====\n";
    MyString a("hello");
    std::cout << "a = " << a.c_str() << ", size = " << a.size() << "\n";

    MyString b = std::move(a);
    std::cout << "b = " << b.c_str() << ", size = " << b.size() << "\n";
    std::cout << "a（被移动后）= \"" << a.c_str() << "\", size = " << a.size() << "\n\n";

    std::cout << "===== 测试 2：拷贝一个被移动后的对象 =====\n";
    MyString c = a;                          // ← 之前这里会 strcpy(dst, nullptr)
    std::cout << "c = \"" << c.c_str() << "\", size = " << c.size() << "\n\n";

    std::cout << "===== 测试 3：普通拷贝 =====\n";
    MyString d = b;
    std::cout << "d = " << d.c_str() << ", size = " << d.size() << "\n";

    return 0;
}