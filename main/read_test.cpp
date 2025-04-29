#include "common/common.h"
#include "circuit/circuit.h"

int main() {
    FILE *cir_file = fopen("../qs_cpp/cpp_qs_test.cir", "r");
    if (cir_file == nullptr) {
        printf("[read_test] open file '../qs_cpp/cpp_qs_test.cir' failed\n");
        return 0;
    }
    Circuit *cir = new Circuit();
    auto size = fread((void *)cir, 114514, 1, cir_file);
    // if (size != 1) {
    //     printf("[read_test] read failed. read size = %d\n", (i32) size);
    //     return 0;
    // }
    printf("cir's name = %s\n", cir->name.c_str());
    return 0;
}