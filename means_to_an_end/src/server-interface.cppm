module;
// Includes go up here, to not conflict with modules
#include <stdint.h>

export module Server;

export namespace Server {

int run(const char* address, const uint16_t port);

}
