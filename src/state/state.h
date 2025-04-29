#pragma once

#include "common/common.h"

class UniMat;

// 量子状态下标三种形式
struct QValue {
    u32 value = 0u;
    // 整型赋值
    QValue(u32 val);
    // 二进制字符串赋值
    QValue(str val);
    // 作为整型使用
    operator u32() const;
    // 返回整型
    u32 u();
    // 返回字符串
    str s(u32 bitnum);
};

// 第0个量子比特当最高位用，\ket{数值}直接与下标同等理解
struct State {
    // 量子比特数
    u32 bitnum;
    // 状态向量
    vec<cf64> coef;
    // 初始化为某值
    State(u32 _bitnum, QValue init_val = 0u);
    // 重置状态
    void reset(QValue init_val = 0u);
    // 按方法打印，输出是状态向量复分布，只能输出全部
    void print_comp_distr(bool keep_zero = true, str name = "", func<str(u32)> f = nullptr, FILE * tar = stdout);
    // 获取active中的比特组成的每一种状态的概率分布
    std::map<QValue, f64> get_prob_distr(vec<u32> active);
    // 按方法打印，输出是active中的比特组成的每一种状态的概率分布
    void print_prob_distr(vec<u32> active, bool keep_zero = true, str name = "", func<str(u32)> f = nullptr);
    // 用方括号直接操作vector
    cf64& operator[](u32 idx);
    // 判断当前状态向量是否满足归一化条件
    bool is_norm_sum_1();
    // 任意酉门运算
    void free_gate(u32 idx, UniMat u);
    // 任意受控门运算
    void ctrl_gate(vec<u32> ctrl, u32 idx, UniMat u);
    // 交换门运算
    void swap_gate(u32 idx1, u32 idx2);
    // 受控交换门运算
    void ctrl_swap_gate(vec<u32> ctrl, u32 idx1, u32 idx2);
    // 测量一部分位
    u32 measure(vec<u32> active);
    // 测量全部位
    u32 measure();
};