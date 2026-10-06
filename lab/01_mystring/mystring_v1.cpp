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
    MyString(const MyString &other)
    {
        size_ = other.size_;             // ① 复制长度
        data_ = new char[size_ + 1];     // ② 申请【新的】内存
        std::strcpy(data_, other.data_); // ③ 复制内容
        std::cout << "[copy-ctor] " << data_ << "\n";
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

    const char *c_str() const { return data_; }
    std::size_t size() const { return size_; }

private:
    char *data_ = nullptr;
    std::size_t size_ = 0;
};

int main()
{
    std::cout << "===== 测试 1：普通赋值 =====\n";
    MyString a("hello");
    MyString c("world");
    c = a;
    std::cout << "c = " << c.c_str() << "   (期望 hello)\n\n";

    std::cout << "===== 测试 2：自赋值 =====\n";
    MyString b("world");
    b = b;
    std::cout << "b = " << b.c_str() << "   (期望 world)\n\n";

    std::cout << "===== 测试 3：链式赋值 =====\n";
    MyString x("X"), y("Y"), z("Z");
    x = y = z;
    std::cout << "x = " << x.c_str() << ", y = " << y.c_str() << "   (期望 Z, Z)\n";

    return 0;
}
