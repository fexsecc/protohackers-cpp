#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>


constexpr size_t ServerBacklog = 50;
constexpr uint16_t ServerPort = 54321;
constexpr size_t TimeoutSeconds = 10;


int main (int argc, char *argv[]) {
    // DoS Protection!!
    alarm(5);

    int32_t ServerSocket = socket(AF_INET, SOCK_DGRAM, 0);
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
        return -1;
    }
    printf("Listening on UDP port %hu...\n", ServerPort);
    socklen_t ClientAddressLen;
    struct sockaddr_in ClientAddress;
    int32_t res;
    uint8_t* EchoBuffer = new uint8_t[65536];
    size_t EchoBufferSize = 65536;

    while(true) {
        res = recvfrom(ServerSocket, EchoBuffer, EchoBufferSize, 0, ( struct sockaddr* ) &ClientAddress, &ClientAddressLen);
        if (res < 0) {
            perror("recvfrom");
            break;
        }
        if (sendto(ServerSocket, EchoBuffer, res, 0, ( struct sockaddr* )&ClientAddress, ClientAddressLen) < 0) {
            perror("sendto");
            break;
        }
    }

    close(ServerSocket);
    delete[] EchoBuffer;
    EchoBuffer = nullptr;
    return 0;
}
