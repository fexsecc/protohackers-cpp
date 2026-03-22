module;

#include <print>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <thread>

module Server;

int Server::init(const char* address, const uint16_t port) {
    // Create socket object
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
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
    }

    return EXIT_SUCCESS;
}


