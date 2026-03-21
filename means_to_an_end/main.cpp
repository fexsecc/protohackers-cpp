#include <print>
#include <string>

int main(int argc, char** argv) {
    const char* server_address;
    uint16_t server_port;
    if (argc < 2) {
        std::println("[*] Using default address and port for bind");
        server_address = "0.0.0.0";
        server_port = 54321;
    }

    std::println("[*] Binding to {}:{}", server_address, server_port);
    return EXIT_SUCCESS;
}
