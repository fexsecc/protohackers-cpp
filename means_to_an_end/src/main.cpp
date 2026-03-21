#include <print>
#include <string>
import server;

int main(int argc, char** argv) {
    const char* server_address;
    uint16_t server_port;
    if (argc < 2) {
        std::println("[*] Using default address and port for bind");
        server_address = "0.0.0.0";
        server_port = 54321;
    }
    else if (argc != 3) {
        std::println("[*] USAGE: ./server <IP> <PORT>");
        return EXIT_FAILURE;
    }
    else {
        server_address = argv[1];
        int32_t temp = std::stoi(argv[2]);
        if (temp <= static_cast<int>(UINT16_MAX) && temp >= 0)
            server_port = static_cast<uint16_t>(temp);
        else {
            std::println("[-] Invalid port number");
            return EXIT_FAILURE;
        }
    }

    std::println("[*] Binding to {}:{}", server_address, server_port);
    server::serve();

    return EXIT_SUCCESS;
}
