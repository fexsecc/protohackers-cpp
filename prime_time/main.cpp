#include <iostream>
#include <vector>
#include <nlohmann/json.hpp>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <signal.h>


constexpr size_t ServerBacklog = 50;
constexpr uint16_t ServerPort = 54321;


int main (int argc, char *argv[]) {
    int32_t ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (ServerSocket < 0) {
        perror("socket");
        return -1;
    }
    sockaddr_in ServerAddress{};
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
            printf("[+] Client connected: %s:%u (pid=%d)\n", AddressBuffer, ntohs(ClientAddress.sin_port), getpid());

            std::vector<char> RequestBuffer;
            char EchoByte;
            while (true) {
                // Read until newline ('\n')
                int32_t res = recv(ClientSocket, &EchoByte, 1, 0);
                if (res < 0) {
                    perror("recv");
                    close(ClientSocket);
                    _exit(-1);
                } else if (!res) {
                    printf("[*] Client disconnected: %s:%u (pid=%d)\n", AddressBuffer, ntohs(ClientAddress.sin_port), getpid());
                    close(ClientSocket);
                    _exit(0);
                }
                if (EchoByte == '\n') {
                    RequestBuffer.push_back('\0');
                    try {
                        auto JsonRequest = nlohmann::json::parse(RequestBuffer.data());
                        std::cout << JsonRequest << std::endl;
                        if (!JsonRequest.contains("method") || !JsonRequest.contains("number") || JsonRequest["method"] != "isPrime") {
                            dprintf(ClientSocket, "Invalid Prime Time format\n");
                            _exit(-1);
                        }
                        if (JsonRequest["number"].type() != nlohmann::json::value_t::number_integer &&
                            JsonRequest["number"].type() != nlohmann::json::value_t::number_unsigned &&
                            JsonRequest["number"].type() != nlohmann::json::value_t::number_float
                        ) {
                            dprintf(ClientSocket, "Invalid number\n");
                            _exit(-1);
                        }
                    } catch (nlohmann::json::parse_error& ex) {
                        std::cerr << "JsonRequest parsing error at byte " << ex.byte << std::endl;
                        dprintf(ClientSocket, "Invalid JSON\n");
                        _exit(-1);
                    }
                    catch (...) {
                        std::cerr << "Unknown exception occurred\n";
                        dprintf(ClientSocket, "Unknown error\n");
                        _exit(-1);
                    }

                    // TODO: Check number primality and return JSON response
                    

                    _exit(0);
                }
                RequestBuffer.push_back(EchoByte);
            }
        }
        else {
            close(ClientSocket);
        }
    }

    return 0;
}
