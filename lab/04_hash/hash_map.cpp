// ═══════════════════════════════════════════════════════════════════
// HashMap<K, V> —— 手写哈希表（拉链法 / Separate Chaining）
//
// 对照 std::unordered_map 的核心功能：
//   put / get / contains / size / bucket_count / load_factor
//
// 核心三件事：
//   ① 哈希函数：把 key 映射到 [0, bucket_count)
//   ② 冲突处理：拉链法 —— 同一桶内的元素串成链表
//   ③ 扩容 rehash：负载因子超过阈值时桶数翻倍，重新散列所有元素
// ═══════════════════════════════════════════════════════════════════
#include <cstddef>
#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

template <typename K, typename V>
class HashMap {
public:
    HashMap() : buckets_(kInitialBuckets, nullptr) {}

    ~HashMap() { free_all_nodes(); }

    // 拷贝构造 / 拷贝赋值：先禁用，避免浅拷贝导致 double free
    HashMap(const HashMap&)            = delete;
    HashMap& operator=(const HashMap&) = delete;

    std::size_t size() const         { return size_; }
    std::size_t bucket_count() const { return buckets_.size(); }

    double load_factor() const {
        return buckets_.empty()
                   ? 0.0
                   : static_cast<double>(size_) / static_cast<double>(buckets_.size());
    }

    // 某个桶里挂了多少个元素（用于观察分布均匀性）
    std::size_t bucket_size(std::size_t i) const {
        std::size_t n = 0;
        for (const Node* p = buckets_[i]; p != nullptr; p = p->next) ++n;
        return n;
    }

    // ───────────────────────────────────────────────────────────
    // ★★★ 任务 1：put —— 插入或更新 ★★★
    //
    //  1. 用 index_of(key) 算出桶下标
    //  2. 在这个桶的链表里找有没有相同的 key
    //       - 找到 → 更新 value，直接 return（size_ 不变）
    //       - 没找到 → 头插法新建节点挂上去，++size_
    //  3. （阶段 2 再加）插入后检查负载因子，必要时 rehash
    // ───────────────────────────────────────────────────────────
    void put(const K& key, const V& value) {
        // TODO: 你的代码
        std::size_t index=index_of(key);
        Node *node=buckets_[index]; 
        while(node){
            if(node->key==key){
            node->value=value;
            return;
            }
            node=node->next;
        }
        Node* newNode=new Node(key,value);
        newNode->next=buckets_[index];
        buckets_[index]=newNode;       
        ++size_;
        if(load_factor()>kMaxLoadFactor){
            rehash(buckets_.size()*2);
        }
    }

    // ───────────────────────────────────────────────────────────
    // ★★★ 任务 2：get —— 查找 ★★★
    //
    //  1. 用 index_of(key) 算出桶下标
    //  2. 遍历桶内链表，找到相同 key 就返回 &(节点的 value)
    //  3. 找不到返回 nullptr
    // ───────────────────────────────────────────────────────────
    V* get(const K& key) {
        std::size_t index=index_of(key);
        Node* node=buckets_[index];
        while(node){
            if(node->key==key){
            return &(node->value);
            }
            node=node->next;
        }
       return nullptr;
    }

    bool contains(const K& key) { return get(key) != nullptr; }

private:
    struct Node {
        K     key;
        V     value;
        Node* next;
        Node(const K& k, const V& v) : key(k), value(v), next(nullptr) {}
    };

    static constexpr std::size_t kInitialBuckets = 8;
    static constexpr double      kMaxLoadFactor  = 0.75;

    // 哈希函数：std::hash 负责把 key 打散成一个 size_t，再取模落到桶上
    std::size_t index_of(const K& key) const {
        return std::hash<K>{}(key) % buckets_.size();
    }

    // ───────────────────────────────────────────────────────────
    // ★★★ 任务 3（阶段 2）：rehash —— 扩容并重新散列 ★★★
    //
    //  1. 保存旧桶数组（std::move 出来）
    //  2. buckets_.assign(newCount, nullptr)  —— 换成更大的空桶数组
    //  3. size_ = 0（因为要重新计数）
    //  4. 遍历旧桶的每个节点，把节点【重新链接】到新桶里
    //       ⚠️ 关键优化：直接搬运节点指针，不要 new/delete 节点
    // ───────────────────────────────────────────────────────────
    void rehash(std::size_t newBucketCount) {
        // TODO（阶段 2）：你的代码
        std::vector<Node*> old=std::move(buckets_);
        buckets_.assign(newBucketCount,nullptr);
        size_=0;
        for(std::size_t i=0;i<old.size();i++){
          Node* node=old[i];
          while(node){
           Node* nextNode=node->next;
           std::size_t index=index_of(node->key);
           node->next=buckets_[index];
           buckets_[index]=node;
           ++size_;
           node=nextNode;
          }
        }
    }

    void free_all_nodes() {
        for (Node* head : buckets_) {
            while (head != nullptr) {
                Node* next = head->next;
                delete head;
                head = next;
            }
        }
        buckets_.assign(buckets_.size(), nullptr);
        size_ = 0;
    }

    std::vector<Node*> buckets_;
    std::size_t        size_ = 0;
};

// ═══════════════════════════════════════════════════════════════════
// 测试驱动
// ═══════════════════════════════════════════════════════════════════
int main() {
    std::cout << "══════ 测试 1：插入与查询 ══════\n";
    {
        HashMap<std::string, int> m;
        for (int i = 0; i < 10; ++i) {
            m.put("key" + std::to_string(i), i * 100);
        }
        std::cout << "size=" << m.size()
                  << "  bucket_count=" << m.bucket_count()
                  << "  load_factor=" << m.load_factor() << "\n";

        int ok = 0, bad = 0;
        for (int i = 0; i < 10; ++i) {
            int* p = m.get("key" + std::to_string(i));
            if (p != nullptr && *p == i * 100) ++ok; else { ++bad; std::cout << "  查不到 key" << i << "\n"; }
        }
        std::cout << "查询结果: 正确 " << ok << " 个，失败 " << bad << " 个\n";
    }

    std::cout << "\n══════ 测试 2：更新已存在的键 ══════\n";
    {
        HashMap<std::string, int> m;
        m.put("a", 1);
        m.put("b", 2);
        m.put("a", 999);              // ← 更新，不是新增
        std::cout << "size=" << m.size() << "   ← 应该是 2，不是 3\n";
        std::cout << "a = " << *m.get("a") << "   ← 应该是 999\n";
        std::cout << "b = " << *m.get("b") << "\n";
    }

    std::cout << "\n══════ 测试 3：查不存在的键 ══════\n";
    {
        HashMap<std::string, int> m;
        m.put("x", 1);
        std::cout << "get(\"nope\") = "
                  << (m.get("nope") == nullptr ? "nullptr  ✓" : "非空  ✗")
                  << "\n";
        std::cout << "contains(\"x\") = " << (m.contains("x") ? "true" : "false") << "\n";
    }

    std::cout << "\n══════ 测试 4：桶分布（拉链法的直观表现）══════\n";
    {
        HashMap<std::string, int> m;
        for (int i = 0; i < 10; ++i) m.put("key" + std::to_string(i), i);

        std::size_t used = 0, maxChain = 0;
        for (std::size_t i = 0; i < m.bucket_count(); ++i) {
            std::size_t n = m.bucket_size(i);
            if (n > 0) ++used;
            if (n > maxChain) maxChain = n;
            std::cout << "  bucket[" << i << "] = " << n << " 个";
            if (n > 1) std::cout << "   ← 冲突，链长度 " << n;
            std::cout << "\n";
        }
        std::cout << "非空桶: " << used << " / " << m.bucket_count()
                  << "   最长链: " << maxChain << "\n";
    }
std::cout << "\n══════ 测试 5：触发 rehash ══════\n";
{
    HashMap<std::string, int> m;
    for (int i = 0; i < 100; ++i) {
        m.put("k" + std::to_string(i), i);
        if (i % 20 == 19) {
            std::cout << "  插入 " << (i + 1) << " 个后: bucket_count="
                      << m.bucket_count() << "  load_factor=" << m.load_factor() << "\n";
        }
    }
    int ok = 0;
    for (int i = 0; i < 100; ++i) {
        int* p = m.get("k" + std::to_string(i));
        if (p && *p == i) ++ok;
    }
    std::cout << "rehash 后全部可查: " << ok << " / 100\n";
}
    std::cout << "\n全部测试完成（阶段 2 会在大量插入时触发 rehash）\n";
    return 0;
}
