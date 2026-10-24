#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <utility>

// ═══════════════════════════════════════════════════════════════
// MyVector<T> —— 手写动态数组容器
//
// 对照 std::vector 的功能：
//   push_back / operator[] / size / capacity / reserve / empty
//
// 注意：这个版本用 new T[] 管理内存，因此要求 T 可默认构造。
//       std::vector 用 operator new + placement new，不要求默认构造 ——
//       那是后续（W7）要改进的点。
// ═══════════════════════════════════════════════════════════════
template <typename T>
class MyVector {
public:
    MyVector() = default;
    //拷贝构造函数
    MyVector(const MyVector& other){
        size_=other.size_;
        capacity_=other.capacity_;
        data_=new T[other.capacity_];
        for(std::size_t i=0;i<size_;i++){
            data_[i]=other.data_[i];
        }
    }
    //移动构造函数
    MyVector(MyVector&& other)noexcept{
    size_=other.size_;
    capacity_=other.capacity_;
    data_=other.data_;
    other.size_=0;
    other.capacity_=0;
    other.data_=nullptr;
    
    }
    //swap函数
    void swap(MyVector& other)noexcept{
    std::swap(this->size_,other.size_);
    std::swap(this->capacity_,other.capacity_);
    std::swap(this->data_,other.data_);
    }
    //赋值运算符
    MyVector& operator=(MyVector other){
    swap(other);
    return *this;
    }
    ~MyVector() { delete[] data_; }

    std::size_t size() const     { return size_; }
    std::size_t capacity() const { return capacity_; }
    bool        empty() const    { return size_ == 0; }

    T&       operator[](std::size_t i)       { return data_[i]; }
    const T& operator[](std::size_t i) const { return data_[i]; }

    // ── 末尾追加元素 ──
    //v.push_back(MyString("hello")) 传的是临时对象，const T& 虽然能绑上，但 data_[size_++] = value 是拷贝赋值——又一次深拷贝。临时对象马上就要销毁，那份拷贝是白花的。
    void push_back(const T& value) {
        if (size_ == capacity_) {
            grow();
        }
        data_[size_++] = value;
    }
    //末尾追加元素，重载，传入右值(临时对象)
    void push_back(T&& value){
        if(size_==capacity_){
            grow();
        }
        data_[size_++]=std::move(value);
    }

    // ── 预分配 ──
    void reserve(std::size_t newCapacity) {
        if (newCapacity <= capacity_) return;
        reallocate(newCapacity);
    }

    void pop_back() {
        if (size_ == 0) return;
        --size_;
    }

private:
    // ───────────────────────────────────────────────────────────
    // TODO：你要实现的只有这一个函数
    //
    // 目标：
    //   ① 计算新容量
    //      - capacity_ == 0 时，起步容量取 4
    //      - 否则新容量 = capacity_ * 2（不超过 2 倍，但必须大于 size_）
    //   ② T* newData = new T[newCapacity];
    //   ③ 把旧元素【移动】到新内存：newData[i] = std::move(data_[i])
    //      旧内存马上要销毁，用 std::move 比复制便宜
    //   ④ delete[] data_;
    //   ⑤ data_ = newData;  capacity_ = newCapacity;
    //
    // 完成后容量应该按 4 → 8 → 16 → 32 增长。
    // 如果每次只 +1，说明写成了 O(n²) 的灾难。
    // ───────────────────────────────────────────────────────────
    void grow() {
        std::size_t newCapacity = (capacity_ == 0) ? 4 : capacity_ * 2;
        reallocate(newCapacity);
    }

    // ───────────────────────────────────────────────────────────
    // ★★★ 你的任务：实现这个函数 ★★★
    //
    // 把容量调整到 newCapacity，并把已有元素搬过去。
    // push_back 扩容和 reserve 预分配都走这里，所以只写一次。
    //
    // 四步：
    //   ① T* newData = new T[newCapacity];
    //   ② 把旧元素【移动】到新内存：
    //        for (i in [0, size_)) newData[i] = std::move(data_[i]);
    //      —— 旧内存马上要删，用 std::move 比复制便宜
    //   ③ delete[] data_;          ← 释放旧内存
    //   ④ data_ = newData;  capacity_ = newCapacity;
    //
    // 想一想：顺序能不能反过来？如果先 delete[] 再搬，会怎样？
    // ───────────────────────────────────────────────────────────
    void reallocate(std::size_t newCapacity) {
        // TODO: 你的代码
        T* newData=new T[newCapacity];
        for(std::size_t i=0;i<size_;i++){
            newData[i]=std::move(data_[i]);
        }
        delete[] data_;
        data_=newData;
        capacity_=newCapacity;
       
    }

    T*          data_     = nullptr;
    std::size_t size_     = 0;
    std::size_t capacity_ = 0;
};
struct Counted {
    static int copies;
    static int moves;

    int v = 0;

    Counted() = default;                       // ①
    explicit Counted(int x) : v(x) {}

    Counted(const Counted& o) : v(o.v)          { ++copies; }   // ② 拷贝构造
    Counted(Counted&& o) noexcept : v(o.v)      { ++moves;  }   // ③ 移动构造

    Counted& operator=(const Counted& o) {                      // ④ 拷贝赋值
        v = o.v;
        ++copies;
        return *this;
    }

    Counted& operator=(Counted&& o) noexcept {                  // ⑤ 移动赋值
        v = o.v;
        ++moves;
        return *this;
    }
};

int Counted::copies = 0;
int Counted::moves  = 0;
// ═══════════════════════════════════════════════════════════════
// 测试驱动
// ═══════════════════════════════════════════════════════════════
int main() {
    std::cout << "══════ 测试 1：push_back 与容量增长 ══════\n";
    {
        MyVector<int> v;
        for (int i = 0; i < 10; ++i) {
            v.push_back(i * 10);
            std::cout << "push_back(" << i * 10 << ")"
                      << "  size=" << v.size()
                      << "  capacity=" << v.capacity() << "\n";
        }

        std::cout << "内容: ";
        for (std::size_t i = 0; i < v.size(); ++i) std::cout << v[i] << " ";
        std::cout << "\n";
    }

    std::cout << "\n══════ 测试 2：reserve 预分配 ══════\n";
    {
        MyVector<int> v;
        v.reserve(100);
        std::cout << "reserve(100) 之后  size=" << v.size()
                  << "  capacity=" << v.capacity() << "\n";
        for (int i = 0; i < 10; ++i) v.push_back(i);
        std::cout << "push 10 个后       size=" << v.size()
                  << "  capacity=" << v.capacity()
                  << "   ← 没有触发扩容\n";
    }

    std::cout << "\n══════ 测试 3：存放自定义类型 ══════\n";
    {
        MyVector<std::pair<int, int>> v;
        v.push_back({1, 2});
        v.push_back({3, 4});
        v.push_back({5, 6});
        std::cout << "pair 容器  size=" << v.size()
                  << "  capacity=" << v.capacity() << "\n";
        for (std::size_t i = 0; i < v.size(); ++i)
            std::cout << "  (" << v[i].first << ", " << v[i].second << ")\n";
    }
    std::cout << "\n══════ 测试 4：拷贝会崩溃（补四件套前）══════\n";
    {
    MyVector<int> a;
    a.push_back(1);
    a.push_back(2);
    std::cout << "a.size() = " << a.size() << "\n";
    MyVector<int> b = a;        // ← 默认拷贝构造 = 浅拷贝
    std::cout << "b.size() = " << b.size() << "\n";
    }   // b 先析构 → delete[] 掉了 a 的数据；a 再析构 → double free
    std::cout << "\n══════ 测试 5：移动构造 ══════\n";
{
    MyVector<int> a;
    for (int i = 0; i < 5; ++i) a.push_back(i);
    std::cout << "移动前: a.size=" << a.size() << " a.capacity=" << a.capacity() << "\n";

    MyVector<int> b = std::move(a);
    std::cout << "移动后: b.size=" << b.size()
              << "  a.size=" << a.size()
              << "  a.capacity=" << a.capacity() << "   ← 应该都是 0\n";

    b.push_back(99);            // b 还能正常用
    std::cout << "b 追加后 size=" << b.size() << "\n";
}
std::cout << "\n══════ 测试 6：push_back 的拷贝 vs 移动 ══════\n";
{
    MyVector<Counted> v;
    v.reserve(10);                    // 预先分配，排除扩容干扰

    Counted c(1);
    v.push_back(c);                   // 传左值 → 拷贝
    std::cout << "push_back(左值) 后:  copies=" << Counted::copies
              << "  moves=" << Counted::moves << "   ← 应该 copies=1\n";

    Counted::copies = Counted::moves = 0;
    v.push_back(Counted(2));          // 传右值 → 移动
    std::cout << "push_back(右值) 后:  copies=" << Counted::copies
              << "  moves=" << Counted::moves << "   ← 应该 moves=1, copies=0\n";
}
std::cout << "\n══════ 测试 7：扩容导致指针失效 ══════\n";
{
    MyVector<int> v;
    for (int i = 0; i < 4; ++i) v.push_back(i * 10);   // 刚好填满 capacity=4

    int* p = &v[0];
    std::cout << "扩容前: &v[0] = " << static_cast<void*>(&v[0])
              << "   p 指向 " << *p << "\n";

    v.push_back(40);                                   // ← 触发扩容

    std::cout << "扩容后: &v[0] = " << static_cast<void*>(&v[0])
              << "   p  仍是 " << static_cast<void*>(p) << "\n";
    std::cout << "→ 地址变了，旧指针 p 已成悬垂指针\n";
}
    std::cout << "\n全部测试完成\n";
    return 0;
}
