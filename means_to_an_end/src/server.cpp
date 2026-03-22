module;

#include <print>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

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
    std::println("[*] Listening for connections...");


    // Create epoll structure from kernel
    int epfd = epoll_create1(0);
    if (epfd < 0) {
        perror("epoll_create1");
        return EXIT_FAILURE;
    }
    // Populate the interest list with our listening socket
    // waiting for
    epoll_event event{};
    event.events = EPOLLIN;
    event.data.fd = sock;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &event) < 0) {
        perror("epoll_ctl");
        return EXIT_FAILURE;
    }

    _Address = address;
    _Port = port;
    _ServerSocket = sock;
    return EXIT_SUCCESS;
}

int Server::start() {

    return EXIT_SUCCESS;
}


