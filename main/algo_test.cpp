#include "algorithm/algorithm.h"
#include "circuit/circuit.h"
#include <bits/types/clock_t.h>

// void iqft(u32 bitnum) {
//     Circuit c(bitnum);
//     u32 t = bitnum;
//     // 先交换
//     for(i32 idx = 0; idx < t - 1 - idx; ++idx) {
//         c.add_gate(Gate(idx, t - 1 - idx));
//     }
//     UniMat R_rev[t + 1];
//     for(u32 k = 2; k <= t; ++k) {
//         R_rev[k] = {
//             1,  0,
//             0,  exp(-2.0 * M_PI * im / (f64)(1u << k))
//         };
//     }
//     for(i32 idx = t - 1; idx >= 0; --idx) {
//         for(i32 ctrl_idx = t - 1; ctrl_idx > idx; --ctrl_idx) {
//             u32 k = ctrl_idx - idx + 1;
//             // 先做Rk
//             c.add_gate(Gate({(u32)ctrl_idx}, idx, R_rev[k], std::format("R_rev[{}]", k)));
//         }
//         // 再做H
//         c.add_gate(Gate(idx, UniMat::H, "H"));
//     }
//     c.run(false);
// }

int main() {
    clock_t cur = clock();
    mono_order_finding(21, 5);
    // iqft(14);
    printf("time: %.6lf\n", (double)(clock() - cur) / (double)CLOCKS_PER_SEC);
    // order_finding(21, 5);
    // shor(114);
    // shor(666);
    // shor(1830); // 用t=L都可以
    // 禁止超过63的数字作为N，其余的靠撞数字干掉。小素因数撞数字的概率相当高，所以应该限制最大的素因数小于64。
    // shor(899); // 29 * 31，不行
    // shor(19 * 23); // 19 * 23
    // shor(19*23*29*31*37);
    // shor(59*59*61);
    // shor(5*7);
    // 记住扩容命令：ulimit -s 10485760，栈空间扩到10G
    // shor(7*11);
}