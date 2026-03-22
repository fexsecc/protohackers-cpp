module;

#include <print>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <thread>
#include <vector>

module Server;

int Server::init(const char* address, const uint16_t port) {
    // Create socket object
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }
    const int enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEADDR)");
        return EXIT_FAILURE;
    }
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEPORT)");
        return EXIT_FAILURE;
    }
    // Format address properly
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    int res = inet_pton(AF_INET, address, &server_addr.sin_addr.s_addr);
    if (res <= 0) {
        if (!res)
            std::println("address not in correct format");
        else
            perror("inet_pton");
        return EXIT_FAILURE;
    }
    server_addr.sin_port = htons(port);

    // Bind address to socket and start listening
    if (bind(sock, (sockaddr*)(&server_addr), sizeof(server_addr)) < 0) {
        perror("bind");
        return EXIT_FAILURE;
    }
    if (listen(sock, 100) < 0) {
        perror("listen");
        return EXIT_FAILURE;
    }

    _Address = address;
    _Port = port;
    _ServerSocket = sock;
    return EXIT_SUCCESS;
}

void Server::HandleConnection(int client_fd) {
    std::vector<stock_msg> prices;
    while(true) {
        stock_msg msg;
        int count = 0;
        // Make sure we read 9 bytes
        while (count < sizeof(msg)) {
            int ret = recv(client_fd, &msg, sizeof(msg) - count, 0);
            // Client closed connection
            if (!ret) {
                std::println("[*] Client closed connection");
                return;
            }
            // An error occurred
            if (ret < 0) {
                perror("recv");
                return;
            }
            count += ret;
        }
        // Ignore unknown message types
        if (msg.Type != 'Q' && msg.Type != 'I')
            continue;
        // Convert our ints to host byte order
        msg.timestamp = ntohl(msg.timestamp);
        msg.price = ntohl(msg.price);
        // Save price and continue
        if (msg.Type == 'I') {
            prices.push_back(msg);
        }
        // Compute mean for 'Q' (query)
        else {
            // A best attempt for now at not overflowing
            int64_t mean;
            if (msg.mintime > msg.maxtime) {
                mean = 0;
            }
            else {
                int32_t in_window_cnt = 0;
                for (auto& price : prices) {
                    if (price.timestamp >= msg.mintime && price.timestamp <= msg.maxtime) {
                        mean += price.price;
                        in_window_cnt += 1;
                    }
                }
                if (in_window_cnt)
                    mean /= in_window_cnt;
                mean = (int32_t)mean;
            }
            int32_t res = htonl(mean);
            send(client_fd, &res, sizeof(int32_t), 0);
        }
    }
}

int Server::start() {
    if (_ServerSocket == -1) {
        std::println("No socket has been found. Did you call init?");
        return EXIT_FAILURE;
    }
    std::println("[*] Accepting connections");
    while (true) {
        struct sockaddr_in client_addr{};
        socklen_t client_addr_len = sizeof(client_addr);
        int client_sock = accept(_ServerSocket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_sock < 0) {
            perror("accept");
            return EXIT_FAILURE;
        }
        char src_addr[40] = {};
        inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, src_addr, sizeof(src_addr));
        std::println("[+] Connection received from {}", src_addr);
        // Service connection with thread
        std::thread conn(&Server::HandleConnection, this, client_sock);
        conn.detach();
    }

    return EXIT_SUCCESS;
}


