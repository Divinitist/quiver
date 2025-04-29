#include "pipe_test.h"

int main() {
    PM::add_pipe("/tmp/test");
    PM::write_to("/tmp/test", "hello world");
    while (true) {

    }
}