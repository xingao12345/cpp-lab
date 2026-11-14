// ═══════════════════════════════════════════════════════════════════
// 实验：进程 vs 线程 —— 地址空间、共享性、创建开销
//
// 三个要验证的问题：
//   ① fork 出来的子进程，和父进程是"同一块内存"还是"副本"？
//   ② 同一进程内的线程，改一个变量，其他线程看得到吗？
//   ③ 创建进程和创建线程，开销差多少？
// ═══════════════════════════════════════════════════════════════════
#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <cstddef>
#include <iostream>
#include <thread>

int main() {
    // ═══════════════════════════════════════════════════════════
    std::cout << "══════ 实验 1：fork —— 地址空间是【副本】══════\n";

    int x = 100;
    std::cout << "fork 之前:  x = " << x << ",  &x = " << &x << "\n";

    // ⚠️ fork 前必须 flush！
    //    fork 会复制 std::cout 的内部缓冲区。如果不 flush，
    //    上面这行文字会被父进程和子进程各打印一次。
    std::cout.flush();

    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "fork 失败\n";
        return 1;
    }

    if (pid == 0) {
        // ── 子进程 ──
        x = 200;                       // 只改自己那份
        std::cout << "  [子进程] pid=" << getpid()
                  << "  x = " << x << ",  &x = " << &x << "\n";
        std::cout.flush();
        _exit(0);                      // ⚠️ 必须用 _exit，不能用 exit
                                       //    exit 会刷新（并复制）父进程的缓冲
    }

    // ── 父进程 ──
    waitpid(pid, nullptr, 0);
    std::cout << "  [父进程] pid=" << getpid()
              << "  x = " << x << ",  &x = " << &x << "\n";
    std::cout << "→ &x 相同（虚拟地址一样），但 x 的值不同\n";
    std::cout << "→ 说明地址空间是【各自一份副本】：写时复制（COW）\n\n";

    // ═══════════════════════════════════════════════════════════
    std::cout << "══════ 实验 2：thread —— 地址空间【共享】══════\n";

    int y = 100;
    std::cout << "创建线程前: y = " << y << ",  &y = " << &y << "\n";

    std::thread t([&y] {
        y = 200;                       // 直接改主线程的变量
        std::cout << "  [子线程] y = " << y << ",  &y = " << &y << "\n";
    });
    t.join();

    std::cout << "  [主线程] y = " << y << ",  &y = " << &y << "\n";
    std::cout << "→ 地址相同、值也被改了 → 同一地址空间，不需要任何进程间通信\n";
    std::cout << "→ 这也意味着：多个线程同时写同一个变量会出问题（下一课的锁）\n\n";

    // ═══════════════════════════════════════════════════════════
    std::cout << "══════ 实验 3：创建开销对比 ══════\n";

    constexpr int N = 300;

    // 创建 + 等待 N 个进程
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < N; ++i) {
        pid_t p = fork();
        if (p == 0) _exit(0);
        waitpid(p, nullptr, 0);
    }
    auto t1 = std::chrono::steady_clock::now();

    // 创建 + 等待 N 个线程
    for (int i = 0; i < N; ++i) {
        std::thread th([] {});
        th.join();
    }
    auto t2 = std::chrono::steady_clock::now();

    double fork_ms   = std::chrono::duration<double, std::milli>(t1 - t0).count();
    double thread_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();

    std::cout << "fork   " << N << " 次: " << fork_ms   << " ms  （"
              << fork_ms   / N * 1000.0 << " us/次）\n";
    std::cout << "thread " << N << " 次: " << thread_ms << " ms  （"
              << thread_ms / N * 1000.0 << " us/次）\n";

    if (thread_ms > 0.0001) {
        std::cout << "→ 线程创建快约 " << (fork_ms / thread_ms) << " 倍\n";
    }
    std::cout << "→ 差距来自：进程要复制页表/地址空间，线程只建栈和寄存器上下文\n\n";

    // ═══════════════════════════════════════════════════════════
    std::cout << "══════ 实验 4：看一下当前进程有多少线程 ══════\n";
    {
        std::cout << "当前进程 pid = " << getpid() << "\n";
        std::cout << "（在另一个终端跑：  ls /proc/" << getpid() << "/task | wc -l）\n";
        std::cout << "（能看到线程数；跑这个程序时可以观察它变化）\n";

        std::thread a([] { std::this_thread::sleep_for(std::chrono::milliseconds(10000)); });
        std::thread b([] { std::this_thread::sleep_for(std::chrono::milliseconds(10000)); });
        std::cout << "现在有 2 个额外线程存活 500ms，去另一个终端数一下\n";
        std::cout.flush();
        a.join();
        b.join();
        std::cout << "两个线程已结束\n";
    }

    std::cout << "\n全部实验完成\n";
    return 0;
}
