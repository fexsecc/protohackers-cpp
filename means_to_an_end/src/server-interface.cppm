module;
// Includes go up here, to not conflict with modules
#include <stdint.h>
#include <string>

export module Server;
import ThreadPool;

export class Server {
    std::string _Address;
    uint16_t _Port;
    int _ServerSocket = -1;
    thread_pool _Pool;
public:
    int init(const char* address, const uint16_t port);
    int start();
};

