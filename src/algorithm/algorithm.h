#pragma once

#include "common/common.h"
#include "circuit/circuit.h"

#define MAX_BITNUM (7)

// 量子求阶算法，返回求到的阶数
u32 order_finding(u32 N, u32 x);
u32 mono_order_finding(u32 N, u32 x);
// 量子质因数分解算法，返回求到的质因数
vec<u32> shor(u32 N);