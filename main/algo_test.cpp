#include "algorithm/algorithm.h"

int main() {
    mono_order_finding(21, 5);
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