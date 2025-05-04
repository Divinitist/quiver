#include "state.h"
#include "uni_mat/uni_mat.h"

QValue::QValue(u32 val) : value(val) {}
QValue::QValue(str val) {
    u32 size = val.length();
    for(u32 i = 0; i < size; ++i) {
        assert(val[i] == '0' || val[i] == '1');
        value |= ((u32)(val[i] == '1') << (size - i - 1));
    }
}

QValue::operator u32() const { return value; }
u32 QValue::u() { return value; }
str QValue::s(u32 bitnum) {
    assert(bitnum <= 32u);
    str ret = "";
    for(u32 i = 0; i < bitnum; ++i) {
        if(value & (1u << i)) {
            ret = "1" + ret;
        } else {
            ret = "0" + ret;
        }
    }
    return ret;
}

State::State(u32 _bitnum, QValue init_val) {
    bitnum = _bitnum;
    assert(bitnum <= 32);
    u32 size = (1u << bitnum);
    for(u32 i = 0; i < size; ++i) coef.push_back(0);
    // 第0位是最高位，所以直接不反转，下标即是状态
    coef[init_val] = 1;
}

void State::reset(QValue init_val) {
    for (u32 i = 0; i < coef.size(); ++i) coef[i] = 0;
    coef[init_val] = 1;
}

void State::print_comp_distr(bool keep_zero, str name, func<str(u32)> f, FILE *tar) {
    if (tar == stdout) {
        printf("+++[%s]+++\n", name.c_str());
    }
    if(f == nullptr) {
        f = [&](u32 num) -> str {
            return u32_to_bin(num, bitnum);
        };
    }
    for(u32 state = 0u; state < coef.size(); ++state) {
        // 第0位是最高位，所以直接不反转，下标即是状态
        if(keep_zero || abs(coef[state]) > 1e-6)
            fprintf(tar, "|%s〉: %s\n", f(state).c_str(), get_complex_str(coef[state]).c_str());
    }
    if (tar == stdout) {
        printf("---[%s]---\n", name.c_str());
    }
}
std::map<QValue, f64>   State::get_prob_distr(vec<u32> active) {
    // 标记active的位置
    u32 active_mask = 0u;
    // ****** 注意这种mask，标记的是比特编号，而比特编号和数值的比特位号是反的 ******
    for(u32 i = 0; i < active.size(); ++i) {
        assert(active[i] <= 32);
        active_mask |= (1u << (bitnum - 1 - active[i]));
    }
    u32 rest_bitnum = bitnum - active.size();
    std::map<QValue, f64> ret;
    for(u32 active_st = 0; active_st < (1u << active.size()); ++active_st) {
        // 对于每个active的状态，统计概率和
        // 制备真实坐标的半成品：把枚举的active状态的比特插到对应的位置上
        u32 semi_real_st = insert_bits(bitnum, ~active_mask, 0u, active_st);
        f64 prob_sum = 0.0;
        for(u32 inactive_st = 0; inactive_st < (1u << rest_bitnum); ++inactive_st) {
            // 制备真实坐标
            u32 real_st = insert_bits(bitnum, active_mask, semi_real_st, inactive_st);
            // 第0位是最高位，所以直接不反转，下标即是状态
            prob_sum += std::norm(coef[real_st]);
        }
        ret[active_st] = prob_sum;
    }
    return ret;
}
void State::print_prob_distr(vec<u32> active, bool keep_zero, str name, func<str(u32)> f) {
    printf("+++[%s]+++\n", name.c_str());
    // 获取概率分布键值对
    auto prob_distr = get_prob_distr(active);
    if(f == nullptr) {
        f = [&](u32 num) -> str {
            return u32_to_bin(num, active.size());
        };
    }
    for(u32 state = 0; state < (1u << active.size()); ++state) {
        if(keep_zero || prob_distr[state] > 1e-6)
            printf("|%s〉: %lf\n", f(state).c_str(), prob_distr[state]);
    }
    printf("---[%s]---\n", name.c_str());
}
cf64 &State::operator[](u32 idx) {
    return coef[idx];
}
bool State::is_norm_sum_1() {
    f64 sum = 0.0;
    for(u32 i = 0; i < (1u << bitnum); ++i) {
        sum += norm(coef[i]);
    }
    // 近似判断相等
    return std::abs(sum - 1.0) < 1e-6;
}

void State::free_gate(u32 idx, UniMat u) {
    assert(idx < bitnum);
    // idx是比特编号，但是此处从头到尾都用的是比特编号，所以不需要反转
    // 推算出来对于多比特系统的酉矩阵为I_{2^k} ⊗ U ⊗ I_{2^{bitnum - 1 - k}}
    // 于是沿着主对角线，每2^{bitnum - k}行重复一次
    /*  a  b
         a  b 
          a  b
        c  d
         c  d
          c  d
    */
    // 对于前2^{bitnum - 1 - k}行，第i个新系数 = a * 第i个旧系数 + b * 第(i + 2^{bitnum - 1 - k})个旧系数
    // 对于后2^{bitnum - 1 - k}行，第i个新系数 = d * 第i个旧系数 + c * 第(i - 2^{bitnum - 1 - k})个旧系数
    u32 block_num = (1u << idx);
    u32 block_size = (1u << (bitnum - idx));
    // 两个协同运算的旧系数之间的下标距离
    u32 gap = (1u << (bitnum - 1 - idx));
    // 由于每个块内都是封闭运算，所以每个块算完就可以覆盖回去，减小内存开销
    // printf("idx = %u, block_size = %u\n", idx, block_size);
    cf64 tmp[block_size];
    // bid是块编号，一共有2^k个块
    for(u32 bid = 0u; bid < block_num; ++bid) {
        // 整体偏移值
        u32 off = (bid << (bitnum - idx));
        // 枚举每一行的子状态
        for(u32 subst = 0u; subst < gap; ++subst) {
            tmp[subst] = u.a * coef[off | subst] + u.b * coef[off | (subst + gap)];
            tmp[subst + gap] = u.c * coef[off | subst] + u.d * coef[off | (subst + gap)];
        }
        // 覆盖回去
        for(u32 subst = 0u; subst < block_size; ++subst) {
            coef[off | subst] = tmp[subst];
        }
    }
}
void State::ctrl_gate(vec<u32> ctrl, u32 idx, UniMat u) {
    u32 ctrl_mask = 0u;
    u32 lower_cnt = 0u;
    for(auto& ctrl_idx : ctrl) {
        // 控制比特与被控比特不能相同
        assert((ctrl_idx < bitnum) && (ctrl_idx != idx));
        // 比特编号对应的数值比特位要反过来
        u32 ctrl_idx_mask = (1u << (bitnum - 1 - ctrl_idx));
        // 确保没有重复的控制比特
        assert((ctrl_mask & ctrl_idx) == 0);
        ctrl_mask |= ctrl_idx_mask;
        // 重排之后，比idx编号小的比特要被去掉，于是idx的编号要前移
        if(ctrl_idx < idx) ++lower_cnt;
    }
    // 去掉比idx编号小的比特，获得重排后的新编号
    u32 new_idx = idx - lower_cnt;
    // 去掉控制比特，获得重排后的比特数
    u32 new_bitnum = bitnum - ctrl.size();
    // 以下操作和单门相同
    u32 block_num = (1u << new_idx);
    u32 block_size = (1u << (new_bitnum - new_idx));
    u32 gap = (block_size >> 1u);
    // 和单门一样，一个block大小的数组就够了
    cf64 tmp[block_size];
    // 枚举块
    for(u32 bid = 0u; bid < block_num; ++bid) {
        u32 off = (bid << (new_bitnum - new_idx));
        // 枚举一半子状态
        for(u32 subst = 0u; subst < gap; ++subst) {
            u32 fullst1 = insert_bits(bitnum, ctrl_mask, ctrl_mask, off | subst);
            u32 fullst2 = insert_bits(bitnum, ctrl_mask, ctrl_mask, off | (subst + gap));
            tmp[subst] = u.a * coef[fullst1] + u.b * coef[fullst2];
            tmp[subst + gap] = u.c * coef[fullst1] + u.d * coef[fullst2];
        }
        // 覆盖回去
        for(u32 subst = 0u; subst < block_size; ++subst) {
            u32 fullst = insert_bits(bitnum, ctrl_mask, ctrl_mask, off | subst);
            coef[fullst] = tmp[subst];
        }
    }
}
void State::swap_gate(u32 idx1, u32 idx2) {
    ctrl_gate({idx1}, idx2, UniMat::X);
    ctrl_gate({idx2}, idx1, UniMat::X);
    ctrl_gate({idx1}, idx2, UniMat::X);
}
void State::ctrl_swap_gate(vec<u32> ctrl, u32 idx1, u32 idx2) {
    // 进行三次Tofolli门
    ctrl.push_back(idx1);
    // 以ctrl和idx1作控制比特
    ctrl_gate(ctrl, idx2, UniMat::X);
    // 把控制比特换成idx2
    ctrl[ctrl.size() - 1] = idx2;
    ctrl_gate(ctrl, idx1, UniMat::X);
    ctrl[ctrl.size() - 1] = idx1;
    ctrl_gate(ctrl, idx2, UniMat::X);
}

u32 State::measure() {
    auto prob = rand01();
    f64 sum = 0.0;
    for(u32 i = 0; i < (1u << bitnum); ++i) {
        sum += std::norm(coef[i]);
        if(sum > prob) return i;
    }
    return (1u << bitnum);
}
u32 State::measure(vec<u32> active) {
    auto prob_distr = get_prob_distr(active);
    auto prob = rand01();
    // printf("prob = %lf\n", prob);
    f64 sum = 0.0;
    for(u32 i = 0; i < prob_distr.size(); ++i) {
        sum += prob_distr[i];
        if(sum > prob) return i;
    }
    return prob_distr.size();
}