#include "algorithm/algorithm.h"

// 判断是否存在a>=1和b>=2满足N=a^b，按照习题5.17
u32 log_factorization(u32 N) {
    // 显然，由于底数是1没意义，底数是2的话b最多是log_2N，所以可以枚举指数b
    // 而消除偶数使得b为2的情形完全没必要讨论
    for(u32 b = 3; (1u << b) <= N; ++b) {
        u32 a = (u32)std::floor(std::pow((f64)N, 1.0 / (f64)b));
        if(pow(a, b) == N) return a;
        if(pow(a + 1, b) == N) return a + 1;
    }
    return 1;
}

vec<u32> shor(u32 N) {
    vec<u32> ret;
    bool has_2 = false;
    while(!(N & 1u)) N >>= 1u, has_2 = true;
    if(has_2) ret.push_back(2);
    printf("[Update] N = %u\n", N);
    u32 cnt = 0;
    while(N > 1) {
        printf("[Round %u]\n", ++cnt);
        u32 a = log_factorization(N);
        if(a > 1) {
            printf("a = %u\n", a);
            ret.push_back(a);
            while(N % a) {
                N /= a;
                printf("[Update] N = %u\n", N);
            }
            continue;
        }
        auto x = rand0r(N - 3) + 2; // x ∈ [2, N - 1]
        printf("x = %u\n", x);
        auto d = gcd(x, N);
        // 如果x与N有公因数，则取掉公因数，继续运算
        if(d > 1) {
            ret.push_back(d);
            N /= d;
            printf("[Update] N = %u\n", N);
            continue;
        }
        auto r = order_finding(N, x);
        if(r == 0) continue;
        // 如果r不是偶数，或者x^(r/2)%N=-1，则再抽
        auto x_r_2 = pow(x, (r >> 1u), N);
        if((r & 1u)) {// || (x_r_2 == N - 1)) {
            printf("[Fallback] r = %u, x^{r/2} = %u\n", r, x_r_2);
            continue;
        }
        auto r1 = gcd(x_r_2 - 1, N);
        auto r2 = gcd(x_r_2 + 1, N);
        if((r1 == 1 && r2 == N) || (r1 == N && r2 == 1) || (r1 == 1 && r2 == 1)) {
            printf("[Fallback] r1 = %u, r2 = %u\n", r1, r2);
            continue;
        }
        if(r1 > 1) {
            ret.push_back(r1);
            N /= r1;
            printf("[Update] r1 = %u, N' = %u\n", r1, N);
        }
        if(r2 > 1) {
            ret.push_back(r2);
            N /= r2;
            printf("[Update] r2 = %u, N' = %u\n", r2, N);
        }
    }
    for(auto& r : ret) {
        printf(" %u", r);
    }
    puts("");
    return ret;
}