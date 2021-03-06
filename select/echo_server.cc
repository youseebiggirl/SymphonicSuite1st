//
// Created by root on 7/17/21.
//

#include "../pkg/net/net.h"
#include <sys/select.h>

#define BUF_SIZE 100

// g++ -o echo_server echo_server.cc ../pkg/net/net.cpp
int main() {
    char buf[BUF_SIZE];
    Server s("tcp", "0.0.0.0", "8080");
    s.Listen(1024);

    fd_set readfds;
    FD_ZERO(&readfds);
    // 监听服务端socket，当有客户连接时会触发事件
    FD_SET(s.Sockfd(), &readfds);

    int maxfd = s.Sockfd();

    for (; ;) {
        // 这句话非常重要，否则会出现多条连接只有一条能正常工作，其他全部阻塞的 bug
        // readfds 保存的是要监听的 fd 集合，但是每次调用 select 后，会将这些监
        // 听 fd 中已经准备好的置 1，未准备好的置 0，如果使用这个已被更改的 fdset，
        // 可能会导致一些异常情况发生，所以这里将 fdset 拷贝一份，调用 select 时传
        // 入拷贝值，这样 select 的更改就不会影响到原 fdset
        fd_set cpyset = readfds;

        // okcnt：准备就绪的 fd 数量
        int okcnt = select(maxfd+1, &cpyset, nullptr, nullptr, nullptr);
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
            if (FD_ISSET(i, &cpyset)) {
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

