#include <iostream>
#include <boost/asio.hpp>
#include <vector>
#include <string>
#include "cli_funcs.h"

using namespace std;

class Session : public std::enable_shared_from_this<Session> {
public:
    explicit Session(boost::asio::io_service &io_service) : _socket(io_service) {}

    boost::asio::ip::tcp::socket &socket() { return _socket; }

    void start() {
        boost::asio::async_read_until(_socket, buffer, '\n',
                                      [this](boost::system::error_code ec, std::size_t) {
                                          if (!ec) handle_request();
                                      });
    }

    void handle_request() {
        // Parse the request from client and perform deduplication
        // Read the command from the buffer
        istream is(&buffer);
        string command;
        getline(is, command);

        // Parse the command into arguments
        vector<string> arguments;
        size_t pos = 0;
        while ((pos = command.find(' ')) != string::npos) {
            arguments.push_back(command.substr(0, pos));
            command.erase(0, pos + 1);
        }
        if (!command.empty()) {
            arguments.push_back(command);
        }

        // Process the command with CLI parser
        try {
            CLI_parser(arguments); 
        } catch (const string &error) {
            // Send error back to client
            string response = error + "\n";
            boost::asio::write(_socket, boost::asio::buffer(response));
            return;
        }

        // Send success message back to client
        string response = "Command executed successfully\n";
        boost::asio::write(_socket, boost::asio::buffer(response));
    }

private:
    boost::asio::ip::tcp::socket _socket; 
    boost::asio::streambuf buffer;
};

class StorageServer {
public:
    StorageServer(int port) : acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) {
        start_accept();
    }

    void start_accept() {
        auto new_session = std::make_shared<Session>(io_service);
        acceptor.async_accept(new_session->socket(),
                              [this, new_session](boost::system::error_code ec) {
                                  if (!ec) new_session->start();
                                  start_accept();
                              });
    }

    void run() { io_service.run(); }

private:
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::acceptor acceptor;
};
