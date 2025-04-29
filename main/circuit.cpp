#include "circuit/circuit.h"

int main() {
    // 用傅里叶变换进行测试
    u32 bitnum, init_val;
    printf("input bitnum: ");       scanf("%u", &bitnum);
    printf("input origin value: "); scanf("%u", &init_val);
    Circuit circuit(bitnum, init_val, "Fourier Transform");
    UniMat R[bitnum + 1];
    for(u32 k = 2; k <= bitnum; ++k) {
        R[k] = {
            1,  0,
            0,  exp(2.0 * M_PI * im / (f64)(1u << k))
        };
    }
    for(u32 idx = 0; idx < bitnum; ++idx) {
        circuit.add_stage(std::format("transform qubit {}", idx));
        circuit.add_gate(Gate(idx, UniMat::H, "H"));
        for(u32 ctrl_idx = idx + 1; ctrl_idx < bitnum; ++ctrl_idx) {
            u32 k = ctrl_idx - idx + 1;
            circuit.add_gate(Gate({ctrl_idx}, idx, R[k], std::format("R({})", k)));
        }
    }
    circuit.add_stage("thorough swap");
    for(i32 idx = 0; idx < bitnum - 1 - idx; ++idx) {
        circuit.add_gate(Gate(idx, bitnum - 1 - idx));
    }
    circuit.run(BY_STAGE);
}