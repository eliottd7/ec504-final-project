#include "network_client_side.h"
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cout << "Usage: " << argv[0] << " <host> <port>" << endl;
        return 1;
    }

    string host = argv[1];
    int port = stoi(argv[2]);

    try {
        DeduplicatorClient client(host, port);
        cout << "Enter commands to send to the server (type 'exit' to quit):" << endl;

        string command;
        while (true) {
            cout << "> ";
            getline(cin, command);
            if (command == "exit") break;

            client.send_command(command);
        }
    } catch (exception &e) {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}