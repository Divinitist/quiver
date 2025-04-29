#pragma once

#include "common/common.h"
#include "state/state.h"
#include "uni_mat/uni_mat.h"

struct Gate {
    // 门名自定义，比如Fredkin和CSWAP是一个门
    str name;
    // 控制比特
    vec<u32> ctrls;
    // 作用比特，不连续的两个表示交换，为空则表示包含全部
    vec<u32> idxs;
    // 门对应的函数，直接操作量子态
    func<void(State&)> f;
    Gate() {};
    // 用酉矩阵构造门，含参数也一样搞。门名和参数这些都没关系，自己重新写，或者后面用parser生成
    Gate(str _name, func<void(State&)> _f);
    // 生成的函数直接去调state的free_gate方法
    Gate(u32 idx, UniMat u, str _name = "Anonymous");
    // 交换门单独写个函数
    Gate(u32 idx1, u32 idx2, str _name = "SWAP");
    // 受控门
    Gate(vec<u32> ctrl, u32 idx, UniMat u, str _name = "CAnonymous");
    // 受控交换门
    Gate(vec<u32> ctrl, u32 idx1, u32 idx2, str _name = "CSWAP");
    // 为qs定制
    Gate(UniMat u, u32 idx): Gate(idx, u) {}
    Gate(UniMat u, str _name, u32 idx): Gate(idx, u, _name) {}
    Gate(UniMat u, u32 idx1, u32 idx2): Gate(idx1, idx2) {}
    Gate(UniMat u, vec<u32> ctrl, u32 idx) : Gate(ctrl, idx, u) {}
    Gate(UniMat u, str _name, vec<u32> ctrl, u32 idx) : Gate(ctrl, idx, u, _name) {}
    Gate(UniMat u, vec<u32> ctrl, u32 idx1, u32 idx2) : Gate(ctrl, idx1, idx2) {}
};

// 阶段
struct Stage {
    str name;
    // 门序列，表示当前阶段要执行的所有门
    vec<Gate> gates;
    Stage(str _name = "") : name(_name) {}
    // 添加门
    void add_gate(Gate);
};

// 运行模式：全部运行，分阶段运行，分门运行
enum RunMode {
    RUSH,
    BY_STAGE,
    BY_GATE
};

struct Circuit {
    str name;
    // 电路中量子系统的状态
    State state;
    // 一系列阶段
    vec<Stage> stages;

    u32 stage_cnt = 0;
    u32 gate_cnt = 0;

    u32 bitnum = 0u;
    u32 init_val = 0u;

    // 初始化量子态，其余不管
    Circuit(u32 num = 0u, QValue val = 0u, str _name = "");

    // 添加门到最新的阶段。若没有阶段则新建一个。
    void add_gate(Gate);
    // 用可变参数构造
    template <typename... Args>
    void emplace_gate(Args&&... args);
    // 添加阶段到电路
    void add_stage(Stage);
    // 添加带名字的空阶段到电路
    void add_stage(str);
    // 输出到指定文件/标准流
    void write_to(u32 bitnum, u32 init_val, str path = "");
    // 从指定目录读取
    void read_from(str path);
    // 运行当前cnt指向的门
    void run_cur_gate();
    // 运行当前cnt指向的阶段
    void run_cur_stage();
    // 运行所有门
    void run_all();
    // 按模式运行，先给出rush下是否需要打印（作为大量子程序的一部分时，不需要输出中间结果）
    void run(bool rush_need_print = false, RunMode run_mode = RUSH);
    // gate_cnt向前移动1
    void gate_cnt_advance();
    // stage_cnt向前移动1
    void stage_cnt_advance();
    // 判断电路是否运行结束
    bool is_end() {
        return stage_cnt == stages.size();
    }
};