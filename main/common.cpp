#include "../src/common/common.h"
#include "../src/state/state.h"

int main() {
    printf("rand01 = %lf\n", rand01());
    printf("rand0r = %u\n", rand0r(100));
    printf("u32 = %u\n", bin_to_u32("1101110"));
    printf("bin = %s\n", u32_to_bin(114514, 17).c_str());
    printf("reverse = %u\n", reverse_bits(114514, 17));
    printf("insert = %u\n", insert_bits(6, 11, 2, 5));
    printf("rad = %lfπ\n", angle_to_rad(45) / M_PI);
    printf("angle = %lf\n", rad_to_angle(M_PI_4));
    printf("z = %s\n", get_complex_str({3.14159, -0.191981}).c_str());
}