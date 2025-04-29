#pragma once

#include "common/common.h"

struct PM {
    static void add_pipe(str name) {
        // 能打开就不新建
        int fd = open(name.c_str(), O_RDWR);
        if (fd == -1) {
            if (mkfifo(name.c_str(), 0666) == -1) {
                perror("mkfifo");
                exit(1);
            }
        }
        close(fd);
    }
    static void write_to(str name, str msg) {
        int fd = open(name.c_str(), O_WRONLY);
        if (fd == -1) {
            perror("open for write");
            exit(1);
        }
        write(fd, msg.c_str(), msg.length() + 1);
        printf("[ppm] wrote '%s' to '%s'\n", msg.c_str(), name.c_str());
        close(fd);
    }
    static str read_from(str name) {
        int fd = open(name.c_str(), O_RDONLY);
        if (fd == -1) {
            perror("open for read");
            exit(1);
        }
        char buf[1024];
        auto size = read(fd, buf, sizeof(buf));
        if (size > 0) {
            buf[size] = '\0';
            printf("[ppm] received '%s' from '%s'\n", buf, name.c_str());
            close(fd);
            return str(buf);
        }
        printf("[ppm] receive from '%s' failed\n", name.c_str());
        close(fd);
        return "";
    }
};