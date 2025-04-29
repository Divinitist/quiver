#include "common.h"

const cf64 im(0, 1);

f64 rand01() {
    // 必须要32位，因为2^{-16}没有精确到1e-6
    u32 rand_num;
    int fd = open("/dev/urandom", O_RDONLY | O_NONBLOCK);
    assert(fd != -1);
    if(read(fd, (void*)&rand_num, 4) != 4) {
        close(fd);
        assert(false);
    }
    close(fd);
    return (f64) rand_num / (f64) UINT32_MAX;
}
u32 rand0r(u32 r) {
    // 运用去尾法
    return (u32) ((f64)(r + 1u) * rand01());
}

u32 bin_to_u32(str num) {
    u32 ret = 0u;
    u32 size = num.length();
    // 依次填补末位，与阅读顺序相同
    for(u32 i = 0; i < size; ++i) {
        ret <<= 1u;
        ret |= (u32)(num[i] - '0');
    }
    return ret;
}
str u32_to_bin(u32 num, u32 bitnum) {
    assert(bitnum <= 32);
    str ret = "";
    for(i32 i = bitnum - 1; i >= 0; --i) {
        // printf("+");
        if(num & (1u << i)) {
            ret += "1";
        } else ret += "0";
    }
    return ret;
}
u32 reverse_bits(u32 origin, u32 bitnum) {
    u32 ret = 0u;
    for(u32 i = 0; i < bitnum; ++i) {
        if(origin & (1u << i)) {
            ret |= (1u << (bitnum - 1 - i));
        }
    }
    return ret;
}
u32 insert_bits(u32 bitnum, u32 mask, u32 origin, u32 patch) {
    u32 ret = origin;
    u32 patch_idx = 0u;
    u32 total_idx = 0u;
    while((1u << patch_idx) <= patch && total_idx < bitnum) {
        // mask位不能填数
        if(!(mask & (1u << total_idx))) {
            if(patch & (1u << patch_idx)) {
                ret |= (1u << total_idx);
            }
            ++patch_idx;
        }
        ++total_idx;
    }
    return ret;
}

f64 angle_to_rad(f64 angle) {
    return angle * M_PIf / 180.0;
}
f64 rad_to_angle(f64 rad) {
    return rad * 180.0 / M_PIf;
}

str get_complex_str(cf64 z) {
    str ret = std::to_string(z.real());
    if(z.imag() < 0.0) {
        ret += " - " + std::to_string(-z.imag()) + "i";
    } else ret += " + " + std::to_string(z.imag()) + "i";
    // printf("debug: %f %f\n", z.real(), z.imag());
    return ret;
}

u32 gcd(u32 a, u32 b) {
    return b ? gcd(b, a % b) : a;
}
u32 pow(u32 x, u32 r) {
    u32 ret = 1u;
    while(r) {
        if(r & 1u) ret = ret * x;
        r >>= 1u;
        x = x * x;
    }
    return ret;
}
u32 pow(u32 x, u32 r, u32 N) {
    u32 ret = 1u;
    while(r) {
        if(r & 1u) ret = ret * x % N;
        r >>= 1u;
        x = x * x % N;
    }
    return ret;
}

bool is_complex_euqal(cf64 x, cf64 y) {
    return abs(x - y) < 1e-6;
}

void PM::add_pipe(str name) {
    // 递归创建文件夹
    str parent = name.substr(0, name.find_last_of('/'));
    system(std::format("mkdir -p {}", parent).c_str());
    // 能以依赖最小的只读模式打开就不新建
    int fd = open(name.c_str(), O_RDONLY);
    if (fd == -1) {
        if (mkfifo(name.c_str(), 0666) == -1) {
            perror("mkfifo");
            exit(1);
        }
    }
    close(fd);
}

void PM::write_to(str name, u8 *msg, ssize_t size) {
    int fd = open(name.c_str(), O_WRONLY);
    if (fd == -1) {
        add_pipe(name);
        fd = open(name.c_str(), O_WRONLY);
        if (fd == -1) {
            perror("open for write");
            exit(1);
        }
    }
    auto write_size = write(fd, msg, size);
    printf("[ppm] wrote %ld bytes to '%s': '", write_size, name.c_str());
    for (auto i = 0; i < write_size; ++i) {
        if (32 <= msg[i] <= 126) printf("%c", msg[i]);
        // hhu代表half half u32
        else printf("\\%u", (u32)msg[i]);
    }
    puts("'");
    close(fd);
}

void PM::write_to(str name, u8 msg) {
    write_to(name, &msg, 1);
}

bool PM::wait_signal(str name) {
    u8 buf;
    read_from(name, &buf, 1);
    return buf ? true : false;
}

void PM::signal(str name) {
    write_to(name, true);
}

void PM::read_from(str name, u8 *buf, ssize_t size) {
    int fd = open(name.c_str(), O_RDONLY);
    if (fd == -1) {
        add_pipe(name);
        fd = open(name.c_str(), O_RDONLY);
        if (fd == -1) {
            perror("open for read");
            exit(1);
        }
    }
    auto read_size = read(fd, buf, size);
    if (read_size > 0) {
        // buf[read_size] = '\0';
        printf("[ppm] received %ld bytes from '%s': '", read_size, name.c_str());
        for (auto i = 0; i < read_size; ++i) {
            if (32 <= buf[i] <= 126) printf("%c", buf[i]);
            // hhu代表half half u32
            else printf("\\%u", (u32)buf[i]);
        }
        puts("'");
    } else {
        printf("[ppm] receive from '%s' failed\n", name.c_str());
    }
    close(fd);
}

void PM::send_cmd(str cmd) {
    write_to("/tmp/quiver/cmd", (u8 *)cmd.c_str(), cmd.length() + 1);
}

void getline(std::ifstream &in, str &s) {
    s = "";
    while (true) {
        std::getline(in, s);
        if (s != "") return;
    }
}