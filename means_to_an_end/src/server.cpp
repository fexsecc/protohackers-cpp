module;

#include <print>
#include <cstdlib>

module Server;

namespace Server {

int serve() {
    std::println("Serving");
    return EXIT_SUCCESS;
}

}


