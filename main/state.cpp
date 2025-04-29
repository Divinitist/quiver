#include "state/state.h"
#include "uni_mat/uni_mat.h"

#define QVALUE_TEST_PASS

int main() {
#ifndef QVALUE_TEST_PASS
    printf("[QValue Test]\n");
    u32 u_val;
    str s_val;

    printf("input u_val: ");
    scanf("%u", &u_val);
    printf("input s_val: ");
    std::cin >> s_val;

    QValue u_q_val = u_val;
    QValue s_q_val = s_val;
    u32 u_val_length = 0u, tmp = u_val;
    while(tmp) {
        ++u_val_length;
        tmp >>= 1u;
    }
    printf("u_val's str = %s\n", u_q_val.s(u_val_length).c_str());
    printf("s_val's u32 = %u\n", s_q_val.u());
#endif
    // State state(7, 114);
    // state.print_comp_distr();
    // state.coef[114] = 0;
    printf("input bitnum: ");
    u32 bitnum;
    scanf("%u", &bitnum);
    State state(bitnum);
    while(true) {
        try {
            printf("input operation: ");
            str gate_name;
            std::cin >> gate_name;
            if(gate_name == "exit") {
                return 0;
            } else if(gate_name == "PROB_DISTR") {
                u32 active_size;
                vec<u32> active;
                scanf("%u", &active_size);
                for(u32 i = 0; i < active_size; ++i) {
                    u32 x;
                    scanf("%u", &x);
                    active.push_back(x);
                }
                state.print_prob_distr(active, true, gate_name);
            } else {
                if(gate_name == "SWAP") {
                    u32 i1, i2;
                    std::cin >> i1 >> i2;
                    state.swap_gate(i1, i2);
                } else if(gate_name == "CSWAP") {
                    u32 num;
                    std::cin >> num;
                    vec<u32> ctrl;
                    for(u32 i = 0; i < num; ++i) {
                        u32 x;
                        std::cin >> x;
                        ctrl.push_back(x);
                    }
                    u32 i1, i2;
                    std::cin >> i1 >> i2;
                    state.ctrl_swap_gate(ctrl, i1, i2);
                } else if(gate_name[0] == 'C') {
                    gate_name = gate_name.substr(1);
                    UniMat u = UniMat::get_uni_mat_by_name(gate_name);
                    u32 num;
                    std::cin >> num;
                    vec<u32> ctrl;
                    for(u32 i = 0; i < num; ++i) {
                        u32 x;
                        std::cin >> x;
                        ctrl.push_back(x);
                    }
                    u32 idx;
                    std::cin >> idx;
                    state.ctrl_gate(ctrl, idx, u);
                } else {
                    u32 idx;
                    std::cin >> idx;
                    UniMat u = UniMat::get_uni_mat_by_name(gate_name);
                    state.free_gate(idx, u);
                }
                state.print_comp_distr(true, gate_name);
                printf("is norm == 1? %s\n", state.is_norm_sum_1() ? "True" : "False");
            }
        } catch(std::exception &e) { // 只有一个抛异常
            printf(
                "usage: <一般门名> <比特编号> \n"
                "       C<一般门名> <控制比特数> <控制比特编号> <受控比特编号>\n"
                "       SWAP <比特编号1> <比特编号2>\n"
                "       CSWAP <控制比特数> <控制比特编号> <受控比特编号1> <受控比特编号2>\n"
            );

        }
    }
}