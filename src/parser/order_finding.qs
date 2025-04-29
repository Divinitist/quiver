v N = 77 // 测试样例
v x = 9
v L = 0
while (2 ^ L < N) {
    L++
}
v t = 2L + 1

bitnum = t + L
init_val = 1

circuit "Order Finding"

stage "1st reg all hadamard"
for i in [0, t) {
    call H @ i
}
stage "apply U"
g U = { // 函数门自带归一化，不用自己写；要带局部变量表，干。
    v x_j = 1
    for j in [0, 2 ^ t) {
        for y in [0, 2 ^ L) {
            assign c'[j * 2 ^ L + x_j * y % N] = self + c[j * 2 ^ L + y]
        }
        assign x_j = x_j * x % N
    }
}
call U
// 写明函数位置，给定操作区间
call "../src/parser/iqft.qs" @ [0 : t - 1]