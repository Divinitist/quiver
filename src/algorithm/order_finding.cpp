#include "algorithm/algorithm.h"

// 注意：此函数遇到测量结果为0的情况会重新测量。在真实的量子计算中，测量结果失效需要重新运行一遍整个量子电路。此处是为了节约时间。
// 结果为0显然是一个正确的解，因为x^0=1显然符合求阶的形式要求，但是是一个平凡解，没有作用。
// 毕竟，对于状态向量完全向外暴露的情况，可以直接通过数峰值数量得到结果。
u32 mono_order_finding(u32 N, u32 x) {
    // 样例：对N=15分解因子，选择与N互质的随机数x=7
    // 于是L=4，t=2L+2=10，总共t+L=14个

    // 初始化为|0000000000>|0001>
    // 这个太特殊了，换成x=5,N=21，阶为6
    u32 L = 0;
    while((1u << L) < N) ++L;
    // u32 t = (L << 1) + 1;
    u32 t = (L << 1u) + 1; // 长度为L勉强能跑，L/2或者3L/4直接寄
    Circuit c(t + L, 1u, "Order Finding");
    c.add_stage("1st reg all hadamard");
    for(int i = 0; i < t; ++i) {
        c.add_gate(Gate(i, UniMat::H, "H"));
    }
    c.add_stage("U");
    c.add_gate(Gate("U", [&](State& state) {
        // 让ctrl_i作为控制比特，控制门是否作用
        // 存放计算结果
        cf64 tmp[state.coef.size()] = {0};
        u32 x_j = 1u;
        // 枚举第一寄存器状态，即|j>
        for(u32 j = 0u; j < (1u << t); ++j) {
            // 枚举第二寄存器状态，即为书中|y>
            for(u32 y = 0u; y < (1u << L); ++y) {
                tmp[(j << L) | (x_j * y % N)] += state[(j << L) | y];
            }
            // 计算x的j次方
            x_j = x_j * x % N;
        }
        // 统计相加后是否满足归一化条件
        f64 norm = 0.0;
        for(u32 i = 0u; i < state.coef.size(); ++i) {
            norm += std::norm(tmp[i]);
        }
        // 覆写回state中
        for(u32 j = 0u; j < (1u << t); ++j) {
            for(u32 y = 0u; y < (1u << L); ++y) {
                state[(j << L) | y] = tmp[(j << L) | y] / norm;
            }
        }
    }));
    c.add_stage("inverse fourier transform @ 1st reg");
    // 先交换
    for(i32 idx = 0; idx < t - 1 - idx; ++idx) {
        c.add_gate(Gate(idx, t - 1 - idx));
    }
    UniMat R_rev[t + 1];
    for(u32 k = 2; k <= t; ++k) {
        R_rev[k] = {
            1,  0,
            0,  exp(-2.0 * M_PI * im / (f64)(1u << k))
        };
    }
    for(i32 idx = t - 1; idx >= 0; --idx) {
        for(i32 ctrl_idx = t - 1; ctrl_idx > idx; --ctrl_idx) {
            u32 k = ctrl_idx - idx + 1;
            // 先做Rk
            c.add_gate(Gate({(u32)ctrl_idx}, idx, R_rev[k], std::format("R_rev[{}]", k)));
        }
        // 再做H
        c.add_gate(Gate(idx, UniMat::H, "H"));
    }
// #define QS_TEST
#ifndef QS_TEST
    c.run();
#else
    c.run(true, BY_STAGE);
#endif
    vec<u32> tar;
    for(u32 i = 0; i < t; ++i) {
        tar.push_back(i);
    }
#ifdef QS_TEST
    c.state.print_prob_distr(tar, false);
    return 0;
#endif
    // c.state.print_prob_distr(tar, false);
    // 出来的结果大概率是0, 1/6, 1/3, 1/2, 2/3, 5/6
    f64 res; 
    do {
        res = (f64)c.state.measure(tar) / (f64)(1u << t);
    } while(res < 1e-6);
    // printf("res = %lf\n", res);
    // 有一定的概率分解出来r=2或3，比较看运气。执行两次相位估计+连分式分解，然后把求出的r求最小公倍数即可得到阶。
    vec<u32> frac_coef;
    for(u32 i = 0; i <= L; ++i) {
        f64 res_fl = std::floor(res);
        // printf("res_fl = %lf\n", res_fl);
        frac_coef.push_back((u32)(res_fl));
        if(std::abs(res - res_fl) < 1e-1) break; // ******** 只要调小误差限制，就能把阶的数字收缩得比较好 ***********
        res = 1.0 / (res - res_fl);
    }
    i32 idx = frac_coef.size() - 1;
    u32 s = 1, r = frac_coef[idx--];
    while(idx >= 0) {
        // printf("s = %u, r = %u\n", s, r);
        s = frac_coef[idx--] * r + s;
        std::swap(r, s);
    }
    std::swap(r, s);
    // printf("s / r = %u / %u\n", s, r);
    return r;
}

u32 order_finding(u32 N, u32 x) {
    // 自己发明的算法：每次order_finding过后，如果结果不是阶，那就再求一次，更新结果为两者最小公倍数，不行就再求。
    if(N >= (1u << MAX_BITNUM)) {
        printf("    [order_finding(%u, %u)] bitnum too large, rejected", N, x);
        return 0;
    }
    printf("    [order_finding(%u, %u)] started\n", N, x);
    auto r = 1;
    // 如果不出结果就一直抽
    while(r > N || pow(x, r, N) != 1u) {
        auto r1 = mono_order_finding(N, x);
        auto r2 = mono_order_finding(N, x);
        r = r1 / gcd(r1, r2) * r2;
        printf("    r1 = %u, r2 = %u, r = %u\n", r1, r2, r);
    }
    printf("    [order_finding(%u, %u)] r = %u\n", N, x, r);
    return r;
}