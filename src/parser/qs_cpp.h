#pragma once

#include "common/common.h"
#include "circuit/circuit.h"

u32 bitnum;
u32 init_val;
str cur_file_name;

void init_qs_server(Circuit &__cir__) {
    Circuit &cir = __cir__;
    cir.write_to(bitnum, init_val, std::format("../qs_cpp/{}.cir", cur_file_name));
    printf("[init_qs_server] file %s.cir generated\n", cur_file_name.c_str());
    // 告知电路文件生成完成
    PM::signal("/tmp/quiver/circuit_file_ready");
}

void deal_circuit_cmd() {
    printf("[qs_server] circuit calculation complete\n");
    PM::signal("/tmp/quiver/circuit_ready");
}

bool qs_server(Circuit &cir) {
    char cmd_c_str[114];
    PM::read_from("/tmp/quiver/cmd", (u8 *)cmd_c_str);
    str cmd(cmd_c_str);
    if (cmd == "off") {
        printf("[qs_server] received cmd 'off'\n");
        return false;
    } else {
        printf("[qs_server] server running...\n");
        if (cmd == "gate") {
            cir.run_cur_gate();
            // printf("[qs_server] here\n");
            deal_circuit_cmd();
        } else if (cmd == "stage") {
            cir.run_cur_stage();
            deal_circuit_cmd();
        } else if (cmd == "all") {
            cir.run_all();
            deal_circuit_cmd();
        } else if (cmd == "reset") {
            cir.state.reset(init_val);
            cir.stage_cnt = cir.gate_cnt = 0;
            deal_circuit_cmd();
        } else if (cmd == "digit") {
            // 输出coef原始值
            State &state = cir.state;
            FILE *digit_data_file = fopen(std::format("../qs_cpp/{}.ddt", cur_file_name).c_str(), "w");
            state.print_comp_distr(false, "", nullptr, digit_data_file);
            fclose(digit_data_file);
            printf("[init_qs_server] file ../qs_cpp/%s.ddt generated\n", cur_file_name.c_str());
            PM::signal("/tmp/quiver/digit_data_ready");
        } else if (cmd == "graph") {
            State &state = cir.state;
            FILE *graph_data_file = fopen(std::format("../qs_cpp/{}.gdt", cur_file_name).c_str(), "w");
            auto lim = std::min(12u, state.bitnum);
            vec<u32> active;
            for (auto i = 0; i < lim; ++i) {
                active.push_back(i);
            }
            auto data = state.get_prob_distr(active);
            for (auto i = 0; i < (1u << lim); ++i) {
                fprintf(graph_data_file, "%lf\n", data[i]);
            }
            fclose(graph_data_file);
            printf("[init_qs_server] file ../qs_cpp/%s.gdt generated\n", cur_file_name.c_str());
            PM::signal("/tmp/quiver/graph_data_ready");
        }
    }
    return true;
}

// 共享内存方案：将整个Circuit的所有内存都共享给客户端，访问指针的时候肯定出错。
// 服务器方案：客户端发请求，服务器执行请求。

// H会有初始化先后问题，导致实际上读到的UniMat::H值换成0
#define qs_begin int main(int argc, char *argv[]) {\
    system("ulimit -s 10485760");\
    auto H = UniMat::H;\
    auto SWAP = UniMat::SWAP;\
    cur_file_name = str(argv[1]);

#define qs_end ; init_qs_server(__cir__); while qs_server(__cir__) begin  end }

#define var     ;auto
#define while   ;while(
#define end     ;}
#define upcount(i, x, y)     ;for(auto i = (x); i <= (y); ++i
#define downcount(i, x, y)   ;for(auto i = (x); i >= (y); --i
#define begin   ) {
#define to      ;
#define in(x, y) 
#define assign  ;
#define circuit      ;Circuit __cir__(bitnum, init_val, 
#define declare     )
#define stage       ;__cir__.add_stage(Stage(
#define start       ))
#define apply        ;__cir__.add_gate(Gate(
#define k_apply(x, y)  ;__cir__.add_gate(Gate(x[y], #x "[" + std::to_string(y) + "]"
#define at(x)          ,x))
#define by(x)       , (vec<u32>){(u32)x}
#define on(x, y)    ,x,y))

#define fgate        ;auto
#define func_begin        [&](State &state) {\
                        cf64 new_state[state.coef.size()] = {0};
#define func_end    upcount (i, 0, state.coef.size() - 1) begin\
                        state[i] = new_state[i];\
                    end\
                    };
#define call(x)        ;__cir__.add_gate(Gate(#x, x))

#define ugate       ;UniMat

// #define run_all         __cir__.run(false)
// #define run_by_stage    __cir__.run(false, BY_STAGE)
// #define run_by_gate     __cir__.run(false, BY_GATE)
// #define show(l, r)      ;vec<u32> tar;\
//                         upcount (i, l, r) begin\
//                             tar.push_back(i);\
//                         end\
//                         __cir__.state.print_prob_distr(tar, false);
