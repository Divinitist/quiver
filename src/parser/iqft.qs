// 功能越人性化，就有越多corner case要处理。时间紧张。
// 行为：定义，赋值，门操作。
circuit "Inverse Quantum Fourier Transform" aka iQFT
v l = 0;
v r = bitnum - 1;
while l < r {
    SWAP@(l, r);
    l++;
    r--;
}
g R'[bitnum + 1];
for k in [2, bitnum] {
    R'[k] = {
        1, 0,
        0, exp(-2 * PI * i / (2 ^ k))
    };
}
for i in (t, 0] {
    for ci in (t, i) {
        v k = ci - i + 1;
        R'[k] ^ ci @ i;
    }
    H @ i;
}
// 句柄突出，注释不可与语句同行
// 所有运算用逆波兰表达式写
// 定义符号-值映射表
// 划分token，然后对于一般表达式做运算，用栈转逆波兰，查符号表计算最终值，然后赋给对应变量
// 
circuit iQFT("Inverse Quantum Fourier Transform")
bitnum = 7 // 作为函数被调用时，会忽略此处的赋值，
init_val = 114 // 作为函数被调用时，会忽略此处的复制
var l = 0
var r = bitnum - 1
while l < r
// 数组全部当变长的来用，直接vector开指定个数，后面的值通过外包到exp()进行返回。
gate R'[bitnum + 1]
// 还是应该全文分析，不过可以用换行符当结尾
// 对于for语句，通过括号匹配找到末尾，然后在代码中创建一个while循环体，每次查询符号表来判断是否跳出
// 然后内部操作递归到别的分析函数去完成
for k in [2, bitnum] {
    // 只允许gate一种特殊结构存在于qs代码中，用assign标记句柄，逗号分开
    assign R'[k] = 1, 0, 0, exp(-2 * PI * i / (2 ^ k))
}
for i in (t, 0] {
    for ci in (t, i) {
        var k = ci - i + 1
        // 直接$号打头
        call R'[k] ^ ci @ i
    }
    call H @ i
}

        