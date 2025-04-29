#include "common/common.h"
#include "circuit/circuit.h"

u32 bitnum;
u32 init_val;

void __output__(Circuit &cir) {
    str full_file_name = std::filesystem::path(__FILE__).filename().string();
    str new_file_name = full_file_name.substr(0, full_file_name.find_last_of('.'));
    str cir_path = "../qs_cpp/" + new_file_name + ".cir";
    FILE *cir_file = fopen(cir_path.c_str(), "w+");
    if(cir_file == nullptr) {
        printf("[__output__] open file '%s' failed\n", cir_path.c_str());
        return;
    }
    auto write_size = fwrite((void *)&cir, sizeof(cir), 1, cir_file);
    if(write_size != 1) {
        printf("[__output__] write failed. written size = %u\n", (u32)write_size);
        return;
    }
}

// H会有初始化先后问题，导致实际上读到的UniMat::H值换成0
#define qs_begin int main() {\
    auto H = UniMat::H;\
    auto SWAP = UniMat::SWAP;
    

#define qs_end  __output__(__cir__);\
                ;}


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

#define run_all         __cir__.run(false)
#define run_by_stage    __cir__.run(false, BY_STAGE)
#define run_by_gate     __cir__.run(false, BY_GATE)
#define show(l, r)      ;vec<u32> tar;\
                        upcount (i, l, r) begin\
                            tar.push_back(i);\
                        end\
                        __cir__.state.print_prob_distr(tar, false);

qs_begin
    var N = 35
    var x = 9
    var L = 0
    while (1 << L) < N begin
        L++
    end
    var t = 2 * L + 1
    assign bitnum = t + L
    assign init_val = 1
    circuit "Order Finding" declare

    stage "1st reg all hadamard" start
    upcount (i, 0, t - 1) begin
        apply H at (i)
    end

    stage "apply U" start
    fgate U = func_begin 
        var x_j = 1
        upcount (j, 0, (1 << t) - 1) begin
            upcount (y, 0, (1 << L) - 1) begin
                assign new_state[(j << L) | (x_j * y % N)] += state[(j << L) | y]
            end
            assign x_j = x_j * x % N
        end
    func_end
    call (U)
    
    stage "iqft" start
    var l = 0
    var r = t - 1
    while l < r - 1 begin
        apply SWAP on (l, r)
        assign l += 1
        assign r -= 1
    end
    ugate Ri[t + 1]
    upcount (k, 2, t) begin
        assign Ri[k] = {
            1, 0,
            0, exp(-2.0 * M_PI * im / ((f64)(1 << k)))
        }
    end
    downcount (i, t - 1, 0) begin
        downcount (ci, t - 1, i + 1) begin
            var k = ci - i + 1
            k_apply (Ri, k) by (ci) at (i)
        end
        apply H at (i)
    end
    // run_by_stage
    // run_all
    // show (0, t - 1)
qs_end