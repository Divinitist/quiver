#pragma once

#include "common/common.h"

// 二阶酉矩阵
struct UniMat {
    cf64 a, b, c, d;
    UniMat(cf64 _a = 0, cf64 _b = 0, cf64 _c = 0, cf64 _d = 0) : a(_a), b(_b), c(_c), d(_d) {}
    // 酉矩阵相乘
    UniMat operator*(const UniMat& y) const;
    // 数乘
    UniMat operator*(const f64 k) const;
    // 判等
    bool operator==(const UniMat &y) const;
    // 打印元素
    void print(str name = "");
    // Pauli-X门
    static UniMat X;
    // Pauli-Y门
    static UniMat Y;
    // Pauli-Z门
    static UniMat Z;
    // Hadamard门
    static UniMat H;
    // 相位S门
    static UniMat S;
    // 相位T门
    static UniMat T;
    // 相位P门，令|1>旋转rad
    static UniMat P(f64 rad);
    // x轴旋转门，令状态在Bloch球x轴上旋转rad
    static UniMat Rx(f64 rad);
    // y轴旋转门，令状态在Bloch球y轴上旋转rad
    static UniMat Ry(f64 rad);
    // y轴旋转门，令状态在Bloch球z轴上旋转rad
    static UniMat Rz(f64 rad);
    // 根据门名得到门实例
    static UniMat get_uni_mat_by_name(str name);
    static str get_name_by_uni_mat(UniMat u);
    
    static UniMat SWAP; // 假门
};
// 浮点数左乘酉矩阵算符
UniMat operator*(const f64 k, const UniMat& u);
