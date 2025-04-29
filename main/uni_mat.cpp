#include "../src/uni_mat/uni_mat.h"

int main() {
    // 赋值测试
    UniMat u(0, im, {3, 4}, 114.514 - 1919.810 * im);
    // 打印测试
    u.print();
    // 反向数乘测试
    UniMat ku = 100.0 * u;
    ku.print();
    UniMat u2 = u * u;
    u2.print();
    UniMat::X.print("X");
    UniMat::Y.print("Y");
    UniMat::Z.print("Z");
    UniMat::H.print("H");
    UniMat::S.print("S");
    UniMat::T.print("T");
    UniMat::P(M_PI_4f).print("P(45)");
    UniMat::Rx(M_PI_2f).print("Rx(90)");
    UniMat::Ry(M_PI_2f).print("Ry(90)");
    UniMat::Rz(M_PI_2f).print("Rz(90)");
}