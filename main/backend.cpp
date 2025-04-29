#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include "common/common.h"

#define PORT 2501
#define BUFFER_SIZE 1024

volatile sig_atomic_t stop = 0;

// 信号处理函数
void handle_signal(int signum) {
    if (signum == SIGINT) {
        std::cout << "Received SIGINT, shutting down server..." << std::endl;
        stop = 1;
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    const char *hello = "Hello from server";

    // 注册信号处理函数
    signal(SIGINT, handle_signal);

    // 创建套接字文件描述符
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 设置套接字选项
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 绑定套接字到指定地址和端口
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    // 监听连接
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    std::cout << "Server listening on port " << PORT << std::endl;

    while (!stop) {
        // 接受客户端连接
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            if (stop) break; // 如果是因为信号中断而停止，跳出循环
            perror("accept");
            continue;
        }
        // 读取客户端发送的数据
        int valread = read(new_socket, buffer, BUFFER_SIZE);
        if (valread > 0) {
            std::cout << "Received from client: " << buffer << std::endl;
            // 前端用qs文件地址请求电路
            auto msg = str(buffer);
            if (msg.find("GET") == 0) {
                str src_path = msg.substr(3);
                
            }
        }
        // 向客户端发送响应
        send(new_socket, hello, strlen(hello), 0);
        std::cout << "Hello message sent" << std::endl;
        // 关闭客户端套接字
        close(new_socket);
    }
    // 关闭服务器套接字
    close(server_fd);
    return 0;
}