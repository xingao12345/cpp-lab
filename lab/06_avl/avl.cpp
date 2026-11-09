// ═══════════════════════════════════════════════════════════════════
// AVL 树 —— 自平衡二叉搜索树
//
// 相比 BST 多出来的东西：
//   ① 每个节点记录 height（子树高度）
//   ② insert/remove 回溯时检查平衡因子，失衡就旋转
//
// 平衡因子 BF = height(left) - height(right)，AVL 要求 BF ∈ {-1, 0, +1}
// 一旦 |BF| >= 2，就需要旋转修复
//
// 四种失衡情况：
//   LL（左左）→ 右旋一次
//   RR（右右）→ 左旋一次
//   LR（左右）→ 先对左孩子左旋，再对当前节点右旋
//   RL（右左）→ 先对右孩子右旋，再对当前节点左旋
// ═══════════════════════════════════════════════════════════════════
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

class AVLTree {
public:
    AVLTree() = default;
    ~AVLTree() { destroy(root_); }

    AVLTree(const AVLTree&)            = delete;
    AVLTree& operator=(const AVLTree&) = delete;

    std::size_t size() const { return size_; }
    int height() const { return node_height(root_); }

    void insert(int key) {
        bool inserted = false;
        root_ = insert_impl(root_, key, inserted);
        if (inserted) ++size_;
    }

    bool contains(int key) const {
        const Node* cur = root_;
        while (cur != nullptr) {
            if (key == cur->key) return true;
            cur = (key < cur->key) ? cur->left : cur->right;
        }
        return false;
    }

    void inorder() const { inorder_impl(root_); std::cout << "\n"; }

    // 检查 AVL 不变式：所有节点 |BF| <= 1，且中序有序
    bool check_invariant() const { return check_impl(root_); }

private:
    struct Node {
        int   key;
        int   height;      // 以本节点为根的子树高度（叶子 = 1）
        Node* left;
        Node* right;
        explicit Node(int k) : key(k), height(1), left(nullptr), right(nullptr) {}
    };

    static int node_height(const Node* n) { return n ? n->height : 0; }

    static void update_height(Node* n) {
        if (n) n->height = 1 + std::max(node_height(n->left), node_height(n->right));
    }

    static int balance_factor(const Node* n) {
        return n ? node_height(n->left) - node_height(n->right) : 0;
    }

    // ───────────────────────────────────────────────────────────
    // ★★★ 任务 1：右旋 ★★★
    //
    //        y                    x
    //       ╱ ╲                  ╱ ╲
    //      x   C    右旋        A   y
    //     ╱ ╲      ────>           ╱ ╲
    //    A   B                    B   C
    //
    // 步骤：
    //   ① Node* x = y->left;
    //   ② Node* B = x->right;
    //   ③ x->right = y;
    //   ④ y->left  = B;
    //   ⑤ 更新高度：先 update_height(y)，再 update_height(x)
    //      ⚠️ 顺序不能反！y 现在是 x 的孩子，必须先更新孩子
    //   ⑥ return x;   ← x 成了新的子树根
    // ───────────────────────────────────────────────────────────
    static Node* rotate_right(Node* y) {
        Node* x=y->left;
        Node* B=x->right;
        y->left=B;
        x->right=y;
        update_height(y);
        update_height(x);
        return x;
    }

    // ───────────────────────────────────────────────────────────
    // ★★★ 任务 2：左旋 ★★★
    //
    //    与右旋完全对称：
    //        x                    y
    //       ╱ ╲                  ╱ ╲
    //      A   y    左旋        x   C
    //         ╱ ╲  ────>       ╱ ╲
    //        B   C            A   B
    //
    // 步骤：
    //   ① Node* y = x->right;
    //   ② Node* B = y->left;
    //   ③ y->left  = x;
    //   ④ x->right = B;
    //   ⑤ 先 update_height(x)，再 update_height(y)
    //   ⑥ return y;
    // ───────────────────────────────────────────────────────────
    static Node* rotate_left(Node* x) {
        Node* y=x->right;
        Node* B=y->left;
        x->right=B;
        y->left=x;
        update_height(x);
        update_height(y);
        return y;
    }

    // ───────────────────────────────────────────────────────────
    // ★★★ 任务 3：rebalance —— 检查并修复失衡 ★★★
    //
    //  ① 先 update_height(node)
    //  ② int b = balance_factor(node)
    //  ③ b > 1（左边重）：
    //       - balance_factor(node->left) >= 0  → LL → return rotate_right(node)
    //       - 否则                              → LR → node->left = rotate_left(node->left);
    //                                                    return rotate_right(node)
    //  ④ b < -1（右边重）：
    //       - balance_factor(node->right) <= 0 → RR → return rotate_left(node)
    //       - 否则                              → RL → node->right = rotate_right(node->right);
    //                                                    return rotate_left(node)
    //  ⑤ 没失衡 → return node
    //
    //  ⚠️ 判断用 >= 0 / <= 0 而不是 == 0：
    //     LL 情况下 bf(left) 可能正好是 0（删除时出现），也必须走单旋
    // ───────────────────────────────────────────────────────────
    static Node* rebalance(Node* node) {
        update_height(node);
        int b=balance_factor(node);
        if(b>1){
            if(balance_factor(node->left)>=0){
                return rotate_right(node);
            }
            else{
                node->left=rotate_left(node->left);
                return rotate_right(node);
            }
        }
        else if(b<-1){
            if(balance_factor(node->right)<=0){
                return rotate_left(node);
            }
            else{
                node->right=rotate_right(node->right);
                return rotate_left(node);
            }
        }
        else{
            return node;
        }
       
    }

    static Node* insert_impl(Node* node, int key, bool& inserted) {
        if (node == nullptr) {
            inserted = true;
            return new Node(key);
        }
        if (key < node->key) {
            node->left = insert_impl(node->left, key, inserted);
        } else if (key > node->key) {
            node->right = insert_impl(node->right, key, inserted);
        } else {
            return node;                       // 重复 key，不插入
        }
        return rebalance(node);                // 回溯路上修复失衡
    }

    static bool check_impl(const Node* node) {
        if (node == nullptr) return true;
        if (std::abs(balance_factor(node)) > 1) return false;
        if (node->height != 1 + std::max(node_height(node->left), node_height(node->right)))
            return false;
        if (node->left  && node->left->key  >= node->key) return false;
        if (node->right && node->right->key <= node->key) return false;
        return check_impl(node->left) && check_impl(node->right);
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
    std::cout << "══════ 测试 1：小规模插入与结构 ══════\n";
    {
        AVLTree t;
        for (int k : {50, 30, 70, 20, 40, 60, 80}) t.insert(k);
        std::cout << "size=" << t.size() << "  height=" << t.height() << "\n";
        std::cout << "中序: "; t.inorder();
        std::cout << "不变式检查: " << (t.check_invariant() ? "通过 ✓" : "失败 ✗") << "\n";
    }

    std::cout << "\n══════ 测试 2：递增插入（BST 会退化成 1000，AVL 应保持 ~10）══════\n";
    {
        AVLTree t;
        for (int i = 1; i <= 1000; ++i) t.insert(i);
        std::cout << "  size   = " << t.size() << "\n";
        std::cout << "  height = " << t.height() << "    ← 应该接近 10\n";
        std::cout << "  不变式 : " << (t.check_invariant() ? "通过 ✓" : "失败 ✗") << "\n";
        std::cout << "  理论下界 log2(1000) = 10\n";
        std::cout << "  理论保证 1.44*log2(n+2) ≈ "
                  << static_cast<int>(1.44 * std::log2(1002)) << "\n";
    }

    std::cout << "\n══════ 测试 3：四种失衡情况必须都触发 ══════\n";
    {
        // LL：插入 30, 20, 10（右旋）
        { AVLTree t; for (int k : {30, 20, 10}) t.insert(k);
          std::cout << "  LL 插入 30,20,10 → height=" << t.height()
                    << "  不变式=" << (t.check_invariant() ? "✓" : "✗") << "\n"; }

        // RR：插入 10, 20, 30（左旋）
        { AVLTree t; for (int k : {10, 20, 30}) t.insert(k);
          std::cout << "  RR 插入 10,20,30 → height=" << t.height()
                    << "  不变式=" << (t.check_invariant() ? "✓" : "✗") << "\n"; }

        // LR：插入 30, 10, 20（先左旋后右旋）
        { AVLTree t; for (int k : {30, 10, 20}) t.insert(k);
          std::cout << "  LR 插入 30,10,20 → height=" << t.height()
                    << "  不变式=" << (t.check_invariant() ? "✓" : "✗") << "\n"; }

        // RL：插入 10, 30, 20（先右旋后左旋）
        { AVLTree t; for (int k : {10, 30, 20}) t.insert(k);
          std::cout << "  RL 插入 10,30,20 → height=" << t.height()
                    << "  不变式=" << (t.check_invariant() ? "✓" : "✗") << "\n"; }
    }

    std::cout << "\n══════ 测试 4：随机插入 1 万个 ══════\n";
    {
        constexpr int N = 10000;
        std::vector<int> keys(N);
        std::iota(keys.begin(), keys.end(), 1);
        std::mt19937 rng(2026);
        std::shuffle(keys.begin(), keys.end(), rng);

        AVLTree t;
        for (int k : keys) t.insert(k);
        std::cout << "  size=" << t.size() << "  height=" << t.height()
                  << "   理论上界 1.44*log2(n+2) ≈ "
                  << static_cast<int>(1.44 * std::log2(N + 2)) << "\n";
        std::cout << "  不变式: " << (t.check_invariant() ? "通过 ✓" : "失败 ✗") << "\n";

        int ok = 0;
        for (int k : keys) if (t.contains(k)) ++ok;
        std::cout << "  全部可查: " << ok << " / " << N << "\n";
    }

    std::cout << "\n全部测试完成\n";
    return 0;
}
