// 一个qs文件是一个电路
circuit "Quantum Fourier Transform" aka QFT // 电路名
bitnum: 7
init_val: 114
v  
g R[bitnum + 1] = (|k| {
    1, 0,
    0, exp(2 * PI * i / 2^k) // 
})
for i: [0, bitnum) {
    stage "transform qubit $i" // 只有在字符串中才需要$符号
    H@i
    for c_i: [i + 1, bitnum) {
        v k = c_i - i + 1
        R[k]^ctrl_i@i
    }
}
stage "thorough swap"
v l = 0
v r = bitnum - 1
while l < r {
    SWAP@(l, r)
    l++
    r--
}