
// ═══════════════════════════════════════════════════════════════════
// BST —— 二叉搜索树（Binary Search Tree）
//
// 性质：对任意节点，左子树所有 key < 该节点 key < 右子树所有 key
//       → 中序遍历得到【严格递增】序列
//       → 查找 / 插入 / 删除 的复杂度 = O(树高 h)
//
// 关键问题：h 的取值范围是 [log₂n, n]。
//          如果按顺序插入，树会【退化成链表】，h = n，所有操作退化为 O(n)。
//          这就是平衡树（AVL / 红黑树）存在的理由。
// ═══════════════════════════════════════════════════════════════════
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

class BST {
public:
    BST() = default;

    ~BST() { destroy(root_); }

    BST(const BST&)            = delete;   // 先禁用拷贝，避免浅拷贝
    BST& operator=(const BST&) = delete;

    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    // ───────────────────────────────────────────────────────────
    // ★★★ 任务 1：insert —— 插入 ★★★
    //
    // 若 key 已存在则不插入（用 contains 先判断一下即可）
    // 否则：root_ = insert_impl(root_, key);  ++size_;
    // ───────────────────────────────────────────────────────────
    void insert(int key) {
        // TODO
        if(contains(key))return;
        root_ = insert_impl(root_, key); 
         ++size_;
    }

    // ───────────────────────────────────────────────────────────
    // ★★★ 任务 2：contains —— 查找 ★★★
    //
    // 从 root_ 往下走：key 小于当前节点就往左，大于就往右，相等就返回 true
    // 走到 nullptr 还没找到 → false
    // ───────────────────────────────────────────────────────────
    bool contains(int key) const {
        // TODO
       Node* cur=root_;
       while(cur){
        if(key<cur->key){
            cur=cur->left;
        }
        else if(key>cur->key){
            cur=cur->right;
        }
        else{
            return true;
        }
       }
        return false;
    }

    // ───────────────────────────────────────────────────────────
    // ★★★ 任务 3：remove —— 删除（本课最难的一个）★★★
    //
    // 先 contains 判断存在，然后：root_ = remove_impl(root_, key); --size_;
    //
    // remove_impl 要处理【三种情况】：
    //   ① 没有左孩子  → 返回右孩子，delete 当前节点
    //   ② 没有右孩子  → 返回左孩子，delete 当前节点
    //   ③ 两个孩子都有 → 用【右子树的最小值】替换当前 key，
    //                    再到右子树里把那个最小值删掉
    //                    （那个最小值一定没有左孩子，所以递归进去会落到情况 ①/②）
    // ───────────────────────────────────────────────────────────
    bool remove(int key) {
        // TODO
        if(contains(key)){
         root_ = remove_impl(root_, key);
          --size_;   
          return true;
        }
        return false;
    }

    // ── 树高（空树为 0，单节点为 1）──
    int height() const { return height_impl(root_); }

    // ── 中序遍历：BST 应输出递增序列 ──
    void inorder() const { inorder_impl(root_); std::cout << "\n"; }

private:
    struct Node {
        int   key;
        Node* left;
        Node* right;
        explicit Node(int k) : key(k), left(nullptr), right(nullptr) {}
    };

    Node* insert_impl(Node* node, int key) {
        if (node == nullptr) return new Node(key);
        if (key < node->key)       node->left  = insert_impl(node->left, key);
        else if (key > node->key)  node->right = insert_impl(node->right, key);
        return node;                       // key 相等：不插入
    }

    Node* remove_impl(Node* node, int key) {
        if (node == nullptr) return nullptr;

        if (key < node->key) {
            node->left = remove_impl(node->left, key);
            return node;
        }
        if (key > node->key) {
            node->right = remove_impl(node->right, key);
            return node;
        }

        // ── 找到目标节点，分三种情况 ──
        if (node->left == nullptr) {                 // ① 只有右孩子（或叶子）
            Node* right = node->right;
            delete node;
            return right;
        }
        if (node->right == nullptr) {                // ② 只有左孩子
            Node* left = node->left;
            delete node;
            return left;
        }
        // ③ 两个孩子都有：用右子树的最小值（中序后继）替换
        Node* successor = min_node(node->right);
        node->key       = successor->key;
        node->right     = remove_impl(node->right, successor->key);
        return node;
    }

    static Node* min_node(Node* node) {
        while (node->left != nullptr) node = node->left;
        return node;
    }

    static int height_impl(const Node* node) {
        if (node == nullptr) return 0;
        return 1 + std::max(height_impl(node->left), height_impl(node->right));
    }

    static void inorder_impl(const Node* node) {
        if (node == nullptr) return;
        inorder_impl(node->left);
        std::cout << node->key << " ";
        inorder_impl(node->right);
    }

    static void destroy(Node* node) {
        if (node == nullptr) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

    Node*       root_ = nullptr;
    std::size_t size_ = 0;
};

// ═══════════════════════════════════════════════════════════════════
int main() {
    std::cout << "══════ 测试 1：基本增删查 ══════\n";
    {
        BST t;
        for (int k : {50, 30, 70, 20, 40, 60, 80}) t.insert(k);
        std::cout << "size=" << t.size() << "  height=" << t.height() << "\n";
        std::cout << "中序遍历（必须递增）: ";
        t.inorder();

        std::cout << "contains(40) = " << (t.contains(40) ? "true" : "false") << "   ← 应为 true\n";
        std::cout << "contains(45) = " << (t.contains(45) ? "true" : "false") << "   ← 应为 false\n";

        t.insert(40);   // 重复插入，size 不应变
        std::cout << "重复 insert(40) 后 size=" << t.size() << "   ← 仍应为 7\n";

        std::cout << "\n-- 删除叶子 20 --\n";
        t.remove(20);
        std::cout << "size=" << t.size() << "  ";
        t.inorder();

        std::cout << "-- 删除只有一个孩子的 30 --\n";
        t.remove(30);
        std::cout << "size=" << t.size() << "  ";
        t.inorder();

        std::cout << "-- 删除有两个孩子的 50（根）--\n";
        t.remove(50);
        std::cout << "size=" << t.size() << "  ";
        t.inorder();

        std::cout << "-- 删除不存在的 999 --\n";
        std::cout << "remove(999) = " << (t.remove(999) ? "true" : "false") << "   ← 应为 false\n";
    }

    constexpr int N = 1000;

    std::cout << "\n══════ 测试 2：顺序插入 → 退化 ══════\n";
    {
        BST t;
        for (int i = 1; i <= N; ++i) t.insert(i);
        std::cout << "插入 1.." << N << "（递增顺序）\n";
        std::cout << "  size = " << t.size() << "\n";
        std::cout << "  height = " << t.height() << "    ← 等于 size，已退化成链表！\n";
    }

    std::cout << "\n══════ 测试 3：随机插入 → 正常 ══════\n";
    {
        std::vector<int> keys(N);
        std::iota(keys.begin(), keys.end(), 1);
        std::mt19937 rng(42);
        std::shuffle(keys.begin(), keys.end(), rng);

        BST t;
        for (int k : keys) t.insert(k);
        std::cout << "插入 1.." << N << "（随机顺序）\n";
        std::cout << "  size = " << t.size() << "\n";
        std::cout << "  height = " << t.height()
                  << "    ← 接近 log₂2000 ≈ 11\n";
    }

    std::cout << "\n══════ 测试 4：查找耗时对比 ══════\n";
    {
        constexpr int LOOKUPS = 20000;

        // 退化的树
        BST degenerate;
        for (int i = 1; i <= N; ++i) degenerate.insert(i);

        // 平衡的树
        std::vector<int> keys(N);
        std::iota(keys.begin(), keys.end(), 1);
        std::mt19937 rng(42);
        std::shuffle(keys.begin(), keys.end(), rng);
        BST balanced;
        for (int k : keys) balanced.insert(k);

        auto bench = [](const BST& t, int n) {
            auto t0 = std::chrono::steady_clock::now();
            volatile int sink = 0;
            for (int i = 0; i < n; ++i) {
                if (t.contains(i % N + 1)) ++sink;
                else --sink;
            }
            auto t1 = std::chrono::steady_clock::now();
            (void)sink;
            return std::chrono::duration<double, std::milli>(t1 - t0).count();
        };

        double d = bench(degenerate, LOOKUPS);
        double b = bench(balanced, LOOKUPS);

        std::cout << "退化的 BST  : " << d << " ms   ← 每次查找平均走 " << (N / 2) << " 步\n";
        std::cout << "平衡的 BST  : " << b << " ms   ← 每次查找平均走约 11 步\n";
        if (b > 0.0001) std::cout << "加速比      : " << (d / b) << " x\n";
    }

    std::cout << "\n全部测试完成\n";
    return 0;
}
