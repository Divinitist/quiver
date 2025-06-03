#include "circuit/circuit.h"

Gate::Gate(str _name, func<void(State&)> _f) : name(_name), f(_f) {}

Gate::Gate(u32 idx, UniMat u, str _name) {
    str u_name = UniMat::get_name_by_uni_mat(u);
    name = (u_name == "" ? _name : u_name);
    idxs.push_back(idx);
    // u.print(name);
    f = [=](State& state) {
        // u.print(std::to_string(idx)); // 为什么这里会是const？
        state.free_gate(idx, u);
        // printf("idx = %d, bitnum = %d\n", idx, state.bitnum);
    };
}
Gate::Gate(u32 idx1, u32 idx2, str _name) {
    name = _name;
    idxs.push_back(idx1);
    idxs.push_back(idx2);
    f = [=](State& state) {
        state.swap_gate(idx1, idx2);
    };
}
Gate::Gate(vec<u32> ctrl, u32 idx, UniMat u, str _name) {
    for(u32 i = 0; i < ctrl.size(); ++i) {
        ctrls.push_back(ctrl[i]);
    }
    idxs.push_back(idx);
    str u_name = UniMat::get_name_by_uni_mat(u);
    name = (u_name == "" ? _name : u_name);
    f = [=](State& state) {
        state.ctrl_gate(ctrl, idx, u);
    };
}
Gate::Gate(vec<u32> ctrl, u32 idx1, u32 idx2, str _name) {
    for(u32 i = 0; i < ctrl.size(); ++i) {
        ctrls.push_back(ctrl[i]);
    }
    idxs.push_back(idx1);
    idxs.push_back(idx2);
    name = _name;
    f = [=](State& state) {
        state.ctrl_swap_gate(ctrl, idx1, idx2);
    };
}

void Stage::add_gate(Gate gate) {
    gates.push_back(gate);
}

Circuit::Circuit(u32 num, QValue val, str _name) : name(_name), state(num, val) {}
void Circuit::add_gate(Gate gate) {
    if(stages.empty()) {
        Stage stage(gate.name + "_stage");
        stage.add_gate(gate);
        stages.push_back(stage);
    } else stages.back().add_gate(gate);
}
template <typename... Args>
void Circuit::emplace_gate(Args&&... args) {
    if(stages.empty()) {
        Stage stage("anonymous_stage");
        stage.gates.emplace_back(std::forward<Args>(args)...);
        stages.push_back(stage);
    } else stages.back().gates.emplace_back(std::forward<Args>(args)...);
}
void Circuit::add_stage(Stage stage) {
    stages.push_back(stage);
}
void Circuit::add_stage(str _name) {
    add_stage(Stage(_name));
}
void Circuit::run(bool rush_need_print, RunMode run_mode) {
    // printf("circuit %s\n", name.c_str());
    for(auto& stage : stages) {
        // printf("stage %s\n", stage.name.c_str());
        for(auto& gate : stage.gates) {
            // printf("gate %s\n", gate.name.c_str());
            gate.f(state);
            if(run_mode == BY_GATE) {
                state.print_comp_distr(false, std::format("[{}] gate '{}' in stage '{}'", name, gate.name, stage.name));
                str cmd;
                while(true) {
                    std::cin >> cmd;
                    if(cmd == "go") break;
                    else if(cmd == "exit") return;
                    else printf("usage: go/exit\n");
                }
            }
        }
        if(run_mode == BY_STAGE) {
            state.print_comp_distr(false, std::format("[{}] stage '{}'", name, stage.name));
            str cmd;
            while(true) {
                std::cin >> cmd;
                if(cmd == "go") break;
                else if(cmd == "exit") return;
                else printf("usage: go/exit\n");
            }
        }
    }
    if(run_mode == RUSH && rush_need_print) {
        state.print_comp_distr(false, std::format("[{}] all complete", name));
    }
}

void Circuit::write_to(u32 bitnum, u32 init_val, str path) {
    // 默认写到stdout
    FILE *f = stdout;
    if (path != "") {
        f = fopen(path.c_str(), "w");
        assert(f != nullptr);
    }
    fprintf(f, "%s\n%u\n%u\n%ld\n", name.c_str(), bitnum, init_val, stages.size());
    for (auto &stage : stages) {
        fprintf(f, "%s\n%ld\n", stage.name.c_str(), stage.gates.size());
        for (auto &gate : stage.gates) {
            fprintf(f, "%s\n", gate.name.c_str());
            fprintf(f, "%ld ", gate.ctrls.size());
            for (auto &ctrl : gate.ctrls) {
                fprintf(f, "%u ", ctrl);
            }
            fprintf(f, "\n");
            fprintf(f, "%ld ", gate.idxs.size());
            for (auto &idx : gate.idxs) {
                fprintf(f, "%u ", idx);
            }
            fprintf(f, "\n");
        }
        fprintf(f, "\n");
    }
    if (path != "") {
        fclose(f);
    }
}

void Circuit::read_from(str path) {
    FILE *f = fopen(path.c_str(), "r");
    assert(f != nullptr);
    char tmp[1024];
    ssize_t stage_size, gate_size, vec_size;
    fscanf(f, "%[^\n]\n%u\n%u\n%ld\n", tmp, &bitnum, &init_val, &stage_size);
    name = str(tmp);
    for (auto i = 0; i < stage_size; ++i) {
        Stage stage;
        fscanf(f, "%[^\n]\n%ld\n", tmp, &gate_size); stage.name = str(tmp);
        for (auto j = 0; j < gate_size; ++j) {
            Gate gate;
            fscanf(f, "%[^\n]\n", tmp); gate.name = str(tmp);
            fscanf(f, "%ld ", &vec_size);
            for (auto k = 0; k < vec_size; ++k) {
                u32 idx;
                fscanf(f, "%u ", &idx);
                gate.ctrls.push_back(idx);
            }
            fscanf(f, "\n");
            fscanf(f, "%ld ", &vec_size);
            for (auto k = 0; k < vec_size; ++k) {
                u32 idx;
                fscanf(f, "%u ", &idx);
                gate.idxs.push_back(idx);
            }
            fscanf(f, "\n");
            stage.add_gate(gate);
        }
        fscanf(f, "\n");
        add_stage(stage);
    }
    fclose(f);
}

void Circuit::run_all() {
    for (auto i = stage_cnt; i < stages.size(); ++i) {
        auto &gates = stages[i].gates;
        for (auto j = gate_cnt; j < gates.size(); ++j) {
            auto &gate = gates[j];
            gate.f(state);
        }
        gate_cnt = 0;
    }
    stage_cnt = stages.size();
    gate_cnt = 0;
}

void Circuit::run_cur_stage() {
    assert(stage_cnt < stages.size());
    // 获取当前阶段
    auto &gates = stages[stage_cnt].gates;
    // 从当前门开始
    for (auto i = gate_cnt; i < gates.size(); ++i) {
        auto &gate = gates[i];
        gate.f(state);
    }
    // 进入新阶段，门计数器归零
    stage_cnt++;
    gate_cnt = 0;
}

void Circuit::run_cur_gate() {
    assert(stage_cnt < stages.size());
    auto &gates = stages[stage_cnt].gates;
    assert(gate_cnt < gates.size());
    auto &gate = gates[gate_cnt];
    gate.f(state);
    gate_cnt++;
    // 进入新阶段
    if (gate_cnt == gates.size()) {
        stage_cnt++;
        gate_cnt = 0;
    }
}

void Circuit::gate_cnt_advance() {
    assert(stage_cnt < stages.size());
    assert(gate_cnt < stages[stage_cnt].gates.size());
    gate_cnt++;
    if (gate_cnt == stages[stage_cnt].gates.size()) {
        stage_cnt++;
        gate_cnt = 0;
    }
}
void Circuit::stage_cnt_advance() {
    assert(stage_cnt < stages.size());
    stage_cnt++;
    gate_cnt = 0;
}