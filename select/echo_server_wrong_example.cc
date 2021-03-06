//
// Created by root on 7/18/21.
//

#include "../pkg/net/net.h"
#include <sys/select.h>

#define BUF_SIZE 100

// g++ -o echo_server_wrong_example echo_server_wrong_example.cc ../pkg/net/net.cpp
// 一个 select 的错误使用案例
int main() {
    char buf[BUF_SIZE];
    Server s("tcp", "0.0.0.0", "8080");
    s.Listen(1024);

    fd_set readfds;
    // 监听服务端socket，当有客户连接时会触发事件
    FD_ZERO(&readfds);
    FD_SET(s.Sockfd(), &readfds);

    int maxfd = s.Sockfd();

    for (; ;) {
        // fd_set cpyset = readfds; // 不使用拷贝
        int okcnt = select(maxfd+1, &readfds, nullptr, nullptr, nullptr);
        printf("already fd num: %d\n", okcnt);
        if (okcnt == -1) {
            printf("select error\n");
            break;
        }
        if (okcnt == 0) {
            continue;
        }

        // 遍历 select 数组
        for (int i = 0; i < maxfd+1; ++i) {
            // 找到所有值为 1 的 fd，这表示该 fd 已经准备就绪了
            if (FD_ISSET(i, &readfds)) {
                // 如果就绪的 fd 为 server,说明有新的连接请求
                if (i == s.Sockfd()) {
                    // 调用 accept 接收客户端的连接请求
                    auto conn = s.Accept();
                    int connfd = conn->Connfd();
                    // 更新 maxfd
                    if (connfd > maxfd) {
                        maxfd = connfd;
                    }
                    // 将 conn 的 fd 添加到 select 监听集合中
                    FD_SET(connfd, &readfds);
                    printf("connected client: %d\n", conn->Connfd());
                    // 如果就绪的 fd 不是 server fd，则是 conn fd
                } else {
                    printf("conn %d is already\n", i);
                    // 可以对这些准备就绪的 conn 进行 echo 处理了
                    int n = read(i, buf, BUF_SIZE);
                    if (n == 0) {   // EOF
                        //
                        FD_CLR(i, &readfds);
                        close(i);
                        printf("close conn: %d\n", i);
                    } else {
                        write(i, buf, n);
                    }
                }
            }
        }
    }

    s.Close();
    return 0;
}

