#include "network_server_side.h"
#include <iostream>
#include <stdexcept>
#include <regex>

using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    int port = std::stoi(argv[1]);

    try {
        StorageServer server(port);
        std::cout << "Server running on port " << port << std::endl;
        server.run();
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}

