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

    ~MyVector() { delete[] data_; }

    std::size_t size() const     { return size_; }
    std::size_t capacity() const { return capacity_; }
    bool        empty() const    { return size_ == 0; }

    T&       operator[](std::size_t i)       { return data_[i]; }
    const T& operator[](std::size_t i) const { return data_[i]; }

    // ── 末尾追加元素 ──
    void push_back(const T& value) {
        if (size_ == capacity_) {
            grow();
        }
        data_[size_++] = value;
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

    std::cout << "\n全部测试完成\n";
    return 0;
}
