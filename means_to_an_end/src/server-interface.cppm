module;
// Includes go up here, to not conflict with modules
#include <stdint.h>
#include <string>

export module Server;

export class Server {
    std::string _Address;
    uint16_t _Port;
    int _ServerSocket = -1;
    void HandleConnection(int client_fd);
public:
    int init(const char* address, const uint16_t port);
    int start();
};

// an unpacked 9 byte struct representing the message
export typedef struct {
    char Type;
    union {
        int32_t timestamp;
        int32_t mintime;
    };
    union {
        int32_t price;
        int32_t maxtime;
    };
} __attribute__((packed)) stock_msg;
