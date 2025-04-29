#pragma once

// 全部STL
// #include <bits/stdc++.h>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <string>
#include <cstdlib>
#include <vector>
#include <map>
#include <complex>
#include <format>
#include <functional>
#include <assert.h>
#include <cstdint>
#include <fstream>
#include <any>
#include <filesystem>
// 系统头文件
#include <fcntl.h>
#include <unistd.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
// 类型命名

using i8 = signed char;
using i16 = signed short;
using i32 = signed int;
using i64 = signed long long;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;

using f32 = float;
using f64 = double;

using cf64 = std::complex<f64>;
using str = std::string;
template <typename T>
using vec = std::vector<T>;
template <typename T>
using func = std::function<T>;

// 常量定义
extern const cf64 im;
// 用于共享内存管理
#define PROJ_ID     (114)

// 随机数函数

// 获取[0,1]范围内随机实数
f64 rand01();
// 获取[0,r]范围内随机非负整数
u32 rand0r(u32 r);

// 二进制函数

// 将二进制数码转为无符号整数
u32 bin_to_u32(str num);
// 将无符号整数转为二进制数码
str u32_to_bin(u32 num, u32 bitnum);
// 翻转比特位
u32 reverse_bits(u32 origin, u32 bitnum);
/**
 * @brief 将patch按位插入到mask以外的origin的位上去
 * 
 * @param bitnum 总比特数
 * @param mask 掩码，其为1的位不允许插入
 * @param origin 原数码
 * @param patch 插入数码
 * @return u32 
 */
u32 insert_bits(u32 bitnum, u32 mask, u32 origin, u32 patch);

// 单位制转换函数

// 角度转弧度
f64 angle_to_rad(f64 angle);
// 弧度转角度
f64 rad_to_angle(f64 rad);

// 输出函数

// 获取复数输出结果
str get_complex_str(cf64 z);

/**
 * @brief 最大公因数
 * 
 * @param a 
 * @param b 
 * @return u32 
 */
u32 gcd(u32 a, u32 b);
u32 pow(u32 x, u32 r);
/**
 * @brief 返回x^r%N的结果
 * 
 * @param x 底数
 * @param r 指数
 * @param N 模数
 * @return u32 
 */
u32 pow(u32 x, u32 r, u32 N);

bool is_complex_euqal(cf64 x, cf64 y);

// 注意：所有发送指示的末尾必须为0以间隔
struct PipeManager {
    static void add_pipe(str name);
    static void write_to(str name, u8 msg);
    static void write_to(str name, u8 *msg, ssize_t size);
    static void read_from(str name, u8 *buf, ssize_t size = 1024);
    static bool wait_signal(str name);
    static void signal(str name);
    static void send_cmd(str cmd);
};

using PM = PipeManager;

// 不断换行直到读入字符
void getline(std::ifstream &in, str &s);