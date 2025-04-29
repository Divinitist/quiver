#include "uni_mat.h"

UniMat UniMat::operator*(const UniMat &y) const {
    return UniMat(
        a * y.a + b * y.c, a * y.b + b * y.d,
        c * y.a + d * y.c, c * y.b + d * y.d
    );
}

UniMat UniMat::operator*(const f64 k) const {
    return UniMat(
        a * k, b * k,
        c * k, d * k
    );
}

void UniMat::print(str name) {
    if(name != "") {
        std::cout << name << ": \n";
    }
    printf("[%s    %s]\n[%s    %s]\n", get_complex_str(a).c_str(), get_complex_str(b).c_str(), get_complex_str(c).c_str(), get_complex_str(d).c_str());
}

UniMat operator*(const f64 k, const UniMat& u) {
    return u * k;
}

UniMat UniMat::X = {0, 1, 1, 0};
UniMat UniMat::Y = {0, -im, im, 0};
UniMat UniMat::Z = {1, 0, 0, -1};
UniMat UniMat::H = UniMat{1, 1, 1, -1} * M_SQRT1_2;
UniMat UniMat::S = {1, 0, 0, im};
UniMat UniMat::T = {1, 0, 0, exp(im * M_PI_4)};
UniMat UniMat::P(f64 rad) { return {1, 0, 0, exp(im * rad)}; }
// 注意R*门的角度都要除以二
UniMat UniMat::Rx(f64 rad) {
    rad /= 2.0;
    return {
        cos(rad), -im * sin(rad),
        -im * sin(rad), cos(rad)
    };
}
UniMat UniMat::Ry(f64 rad) {
    rad /= 2.0;
    return {
        cos(rad), -sin(rad),
        -sin(rad), cos(rad)
    };
}
UniMat UniMat::Rz(f64 rad) {
    rad /= 2.0;
    return {
        exp(-im * rad), 0,
        0, exp(im * rad)
    };
}
UniMat UniMat::SWAP = {1, 0, 1, 0};

// 连着参数一起塞进去
UniMat UniMat::get_uni_mat_by_name(str name) {
    if(name == "X") return X;
    if(name == "Y") return Y;
    if(name == "Z") return Z;
    if(name == "H") return H;
    if(name == "S") return S;
    if(name == "T") return T;
    u32 l = 0, r = 0;
    for(u32 i = 0; i < name.length(); ++i) {
        if(name[i] == '(') l = i + 1;
        if(name[i] == ')') r = i - 1;
    }
    if(!(l && r)) goto fallback;
    else {
        f64 param = std::stod(name.substr(l, r - l + 1));
        f64 rad = angle_to_rad(param);
        if(name[0] == 'P') return P(rad);
        if(!(name[0] == 'R')) goto fallback;
        if(name[1] == 'x') return Rx(rad);
        if(name[1] == 'y') return Ry(rad);
        if(name[1] == 'z') return Rz(rad);
    }
fallback:
    // 抛出异常，让上层处理
    throw std::exception();
    return UniMat();
}

bool UniMat::operator==(const UniMat &y) const {
    return is_complex_euqal(a, y.a) && 
           is_complex_euqal(b, y.b) &&
           is_complex_euqal(c, y.c) &&
           is_complex_euqal(d, y.d);
}

str UniMat::get_name_by_uni_mat(UniMat u) {
    if(u == X) return "X";
    if(u == Y) return "Y";
    if(u == Z) return "Z";
    if(u == H) return "H";
    if(u == S) return "S";
    if(u == T) return "T";
    return "";
}
