module;
// Includes go up here, to not conflict with modules
#include <stdint.h>

export module Server;

export class Server {
    char* _Address;
    uint16_t _Port;
    int _ServerSocket;
public:
    int init(const char* address, const uint16_t port);
};

