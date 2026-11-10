// ═══════════════════════════════════════════════════════════════════
// B 树 / B+ 树 为什么适合磁盘 —— 用数字说话
//
// 核心矛盾：
//   内存里一次比较 ≈ 1 纳秒；磁盘上一次随机读 ≈ 10 毫秒 = 1000 万倍
//   所以树结构在磁盘上的评价标准不是"比较次数"，而是"IO 次数"
//
// 关键洞察：
//   二叉树每个节点只存 1 个 key → 每个节点都要单独一次 IO
//   B+ 树一个节点（一页 4KB）能存几百个 key → 一次 IO 排掉几百个分支
// ═══════════════════════════════════════════════════════════════════
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <string>

namespace {

constexpr double kSeekMs  = 10.0;    // 一次随机磁盘寻道 + 读取 ≈ 10 ms
constexpr double kNanoMs  = 1e-6;    // 1 纳秒 = 0.000001 毫秒

// 一个节点能容纳多少个 key（扇出）
std::size_t fanout_for(std::size_t page_bytes, std::size_t entry_bytes) {
    return page_bytes / entry_bytes;
}

// 以 fanout 为扇出的树，装 n 条记录需要多少层（含根，不含下一层）
std::size_t tree_height(std::size_t n, std::size_t fanout) {
    std::size_t h = 0;
    std::size_t nodes = n;
    do {
        ++h;
        nodes = (nodes + fanout - 1) / fanout;   // 向上取整
    } while (nodes > 1);
    return h;
}

void print_row(const std::string& name,
               std::size_t height,
               double ms_per_lookup) {
    std::cout << std::left  << std::setw(28) << name
              << std::right << std::setw(8)  << height
              << std::setw(14) << ms_per_lookup
              << "\n";
}

}  // namespace

int main() {
    constexpr std::size_t kRecords  = 1'000'000;   // 100 万条记录
    constexpr std::size_t kPageSize = 4096;        // 磁盘页 4KB

    std::cout << "══════ 场景设定 ══════\n";
    std::cout << "记录数        : " << kRecords << " 条\n";
    std::cout << "磁盘页大小    : " << kPageSize << " 字节\n";
    std::cout << "一次随机 IO   : " << kSeekMs << " ms\n\n";

    // ── 每条记录占 16 字节（8 字节 key + 8 字节指针/值）──
    constexpr std::size_t kEntryBytes = 16;

    std::cout << "══════ 一页能放多少个 key ══════\n";

    // 二叉树：一个节点只有 1 个 key + 2 个指针（约 16~24 字节）
    // 但节点是分散分配的，一次查找的每个节点都可能在不同页
    const std::size_t bst_fanout = 2;   // 每个节点 2 个分支
    const std::size_t bplus_fanout = fanout_for(kPageSize, kEntryBytes);

    std::cout << "二叉树（每节点 1 个 key）: 扇出 " << bst_fanout << "\n";
    std::cout << "B+ 树（4KB 页塞满）      : 扇出 " << bplus_fanout
              << "  (= " << kPageSize << " / " << kEntryBytes << ")\n\n";

    std::cout << "══════ 树高与查找 IO ══════\n";
    std::cout << std::left  << std::setw(28) << "结构"
              << std::right << std::setw(10) << "树高"
              << std::setw(18) << "每次查找 IO"
              << std::setw(18) << "估算毫秒"
              << "\n";
    std::cout << std::string(74, '-') << "\n";

    // ① 二叉树（含红黑树）：每个节点一次 IO
    {
        std::size_t h = static_cast<std::size_t>(
            std::ceil(std::log2(static_cast<double>(kRecords))));
        print_row("二叉树 / 红黑树", h, h * kSeekMs);
    }

    // ② B+ 树：3 层就够了
    {
        std::size_t h = tree_height(kRecords, bplus_fanout);
        print_row("B+ 树（4KB 页，16B/key）", h, h * kSeekMs);
    }

    // ③ 假如 key 更小（8 字节），扇出翻倍
    {
        std::size_t fo = fanout_for(kPageSize, 8);
        std::size_t h  = tree_height(kRecords, fo);
        print_row("B+ 树（8B/key，扇出 " + std::to_string(fo) + "）", h, h * kSeekMs);
    }

    // ④ 对比：数据全在内存里（无 IO）
    {
        double ms = std::ceil(std::log2(static_cast<double>(kRecords))) * 20 * kNanoMs;
        print_row("（对比）全内存 B+ 树", 3, ms);
    }

    std::cout << "\n══════ 结论 ══════\n";
    std::cout << "· 二叉树查找 100 万条要 ~20 次 IO = ~200 ms\n";
    std::cout << "· B+ 树同样数据只要 3 次 IO = ~30 ms\n";
    std::cout << "· 差距约 " << (20 * kSeekMs) / (3 * kSeekMs) << " 倍，而且数据量越大差距越大\n\n";

    std::cout << "关键不是「比较次数少」，而是：\n";
    std::cout << "  一个 4KB 页能装 " << bplus_fanout
              << " 个 key → 一次 IO 排掉 " << bplus_fanout << " 个分支\n";
    std::cout << "  二叉树一个节点只有 1 个 key → 一次 IO 只能排掉 1 个分支\n\n";

    // ── 为什么 B+ 树的叶子要存全部数据？──
    std::cout << "══════ 为什么 B+ 树的叶子还要存一份完整数据 ══════\n";
    const std::size_t leaf_pages = (kRecords + bplus_fanout - 1) / bplus_fanout;
    std::cout << "· 100 万条记录 → 叶子页约 " << leaf_pages << " 个\n";
    std::cout << "· 叶子用双向链表串起来 → 范围查询（BETWEEN / ORDER BY）\n";
    std::cout << "  只需找到起点，然后顺序扫叶子，不用回到根重新找\n";
    std::cout << "· B 树的 key 只出现一次，范围查询要在树里反复上下跳\n";
    std::cout << "  → 这就是数据库选 B+ 树、不选 B 树的核心原因之一\n";

    return 0;
}
