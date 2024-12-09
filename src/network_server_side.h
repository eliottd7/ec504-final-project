#include "cli_funcs.h"
#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Session {
public:
    explicit Session(boost::asio::ip::tcp::socket socket) : _socket(std::move(socket)) {
    }

    void start() {
        std::cout << "Session started, waiting for data..." << std::endl;
        while ( true ) {
            try {
                // Read data synchronously
                std::string data = read_data();
                if ( data.empty() ) {
                    std::cout << "Client closed the connection." << std::endl;
                    break;
                }

                std::cout << "Raw data: '" << data << "'" << std::endl;
                handle_request(data);
            } catch ( const std::exception& e ) {
                std::cerr << "Error: " << e.what() << std::endl;
                break;
            }
        }
    }

private:
    boost::asio::ip::tcp::socket _socket;

    std::string read_data() {
        boost::asio::streambuf buffer;
        boost::asio::read_until(_socket, buffer, '\n'); // Synchronous read
        std::istream is(&buffer);
        std::string data;
        std::getline(is, data);
        return data;
    }

    void handle_request(const std::string& command) {
        try {
            // Parse the command
            std::vector<std::string> arguments;
            std::stringstream ss(command);
            std::string arg;
            while ( ss >> arg ) {
                arguments.push_back(arg);
            }

            // Process the command with CLI parser
            CLI_parser(arguments);

            // Send success message back to the client
            if ( _socket.is_open() ) {
                std::string response = "Command executed successfully\n";
                boost::asio::write(_socket, boost::asio::buffer(response));
            }
        } catch ( const std::string& error ) {
            // Send error back to the client
            if ( _socket.is_open() ) {
                boost::asio::write(_socket, boost::asio::buffer(error + "\n"));
            }
        } catch ( const std::exception& e ) {
            // Handle unexpected exceptions
            if ( _socket.is_open() ) {
                std::string error = "ERROR: Unexpected exception: ";
                error += e.what();
                error += "\n";
                boost::asio::write(_socket, boost::asio::buffer(error));
            }
        }
    }
};

class StorageServer {
public:
    explicit StorageServer(int port) : acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) {
    }

    void run() {
        try {
            std::cout << "Waiting for a client to connect..." << std::endl;

            // Accept a single connection synchronously
            boost::asio::ip::tcp::socket socket(io_service);
            acceptor.accept(socket); // Synchronous accept
            std::cout << "New connection accepted" << std::endl;

            // Handle the session
            Session session(std::move(socket));
            session.start();

        } catch ( const std::exception& e ) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

private:
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::acceptor acceptor;
};
