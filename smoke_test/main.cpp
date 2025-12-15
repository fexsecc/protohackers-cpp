#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


constexpr size_t ServerBacklog = 50;
constexpr uint16_t ServerPort = 54321;


int main (int argc, char *argv[]) {

    int32_t ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (ServerSocket < 0) {
        perror("socket");
        return -1;
    }
    sockaddr_in ServerAddress;
    ServerAddress.sin_addr.s_addr = INADDR_ANY;
    ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_port = htons(ServerPort);
    if (bind(ServerSocket, (struct sockaddr*)&ServerAddress, sizeof(ServerAddress)) < 0) {
        perror("bind");
        close(ServerSocket);
        return -1;
    }
    if (listen(ServerSocket, ServerBacklog) < 0) {
        perror("listen");
        close(ServerSocket);
        return -1;
    }
    printf("Listening on TCP port %hu...\n", ServerPort);
    while (true) {
        struct sockaddr_in ClientAddress{};
        socklen_t ClientAddressLen = sizeof(ClientAddress);
        int32_t ClientSocket = accept(ServerSocket, (struct sockaddr*)&ClientAddress, &ClientAddressLen);
        if (ClientSocket < 0) {
            perror("accept");
            close(ServerSocket);
            return -1;
        }
        pid_t ClientPid = fork();
        if (ClientPid < 0) {
            perror("fork");
            close(ServerSocket);
            return -1;
        }
        else if (!ClientPid) {
            // child
            close(ServerSocket);
            char AddressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &ClientAddress.sin_addr, AddressBuffer, sizeof(AddressBuffer));
            printf("[+] Client connected: %s:%u (pid=%d)\n", AddressBuffer, ntohs(ClientAddress.sin_port), ClientPid);
            uint8_t EchoByte;
            while (true) {
                int32_t res = recv(ClientSocket, &EchoByte, 1, 0);
                if (res < 0) {
                    perror("recv");
                    close(ClientSocket);
                    _exit(-1);
                } else if (!res) {
                    printf("[*] Client disconnected: %s:%u (pid=%d)\n", AddressBuffer, ntohs(ClientAddress.sin_port), ClientPid);
                    close(ClientSocket);
                    _exit(0);
                }
                write(ClientSocket, &EchoByte, 1);
            }
        }
        else {
            close(ClientSocket);
        }
    }

    return 0;
}
