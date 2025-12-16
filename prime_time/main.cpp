#include <iostream>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cstdint>
#include <inttypes.h>


constexpr size_t ServerBacklog = 50;
constexpr uint16_t ServerPort = 54321;


class MillerRabinTest {
    // modular multiply (avoid overflow) using __int128
    static uint64_t modmul(uint64_t a, uint64_t b, uint64_t m) {
        return (unsigned __int128)a * b % m;
    }
    
    static uint64_t modpow(uint64_t a, uint64_t d, uint64_t m) {
        uint64_t res = 1;
        while (d) {
            if (d & 1) res = modmul(res, a, m);
            a = modmul(a, a, m);
            d >>= 1;
        }
        return res;
    }
    
public:
    bool isPrime64(uint64_t n) {
        if (n < 2) return false;
        static const uint64_t small_primes[] = {2,3,5,7,11,13,17,19,23,29,31,37};
        for (uint64_t p : small_primes) {
            if (n == p) return true;
            if (n % p == 0) return false;
        }
        // write n-1 as d * 2^s
        uint64_t d = n - 1;
        int s = 0;
        while ((d & 1) == 0) { d >>= 1; ++s; }
    
        // Deterministic bases for 64-bit integers
        uint64_t bases[] = {2ULL, 3ULL, 5ULL, 7ULL, 11ULL, 13ULL, 17ULL, 19ULL, 23ULL};
        for (uint64_t a : bases) {
            if (a % n == 0) continue;
            uint64_t x = modpow(a, d, n);
            if (x == 1 || x == n - 1) continue;
            bool composite = true;
            for (int r = 1; r < s; ++r) {
                x = modmul(x, x, n);
                if (x == n - 1) { composite = false; break; }
            }
            if (composite) return false;
        }
        return true;
    }
};

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

                        nlohmann::json JsonResponse;
                        JsonResponse["method"] = "isPrime";
                        if (JsonRequest["number"].type() == nlohmann::json::value_t::number_float ||
                            JsonRequest["number"] < 2) {
                            JsonResponse["prime"] = false;
                            std::string Payload = JsonResponse.dump();
                            dprintf(ClientSocket, "%s\n", Payload.c_str());
                            _exit(0);
                        }
                        MillerRabinTest PrimeTest;
                        if (PrimeTest.isPrime64(JsonRequest["number"])) {
                            JsonResponse["prime"] = true;
                            std::string Payload = JsonResponse.dump();
                            dprintf(ClientSocket, "%s\n", Payload.c_str());
                            _exit(0);
                        } else {
                            JsonResponse["prime"] = false;
                            std::string Payload = JsonResponse.dump();
                            dprintf(ClientSocket, "%s\n", Payload.c_str());
                            _exit(0);
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
