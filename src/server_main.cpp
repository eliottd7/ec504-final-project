#include "network_server_side.h"
#include <iostream>


using namespace std; 

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <port>" << endl;
        return 1;
    }

    int port = stoi(argv[1]);

    try {
        StorageServer server(port);
        cout << "Server running on port " << port << endl;
        server.run();
    } catch (exception &e) {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}

