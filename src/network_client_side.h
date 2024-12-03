#include <iostream>
#include <boost/asio.hpp>
#include <string>

using namespace std;
using boost::asio::ip::tcp;

class DeduplicatorClient {
public:
    DeduplicatorClient(const string &host, int port)
        : io_service(), socket(io_service) {
        // Resolve the host and port
        tcp::resolver resolver(io_service);
        auto endpoint_iterator = resolver.resolve(host, to_string(port));
        
        // Connect to the server
        boost::asio::connect(socket, endpoint_iterator);
    }

    void send_command(const string &command) {
        // Send the command to the server
        string msg = command + "\n";
        boost::asio::write(socket, boost::asio::buffer(msg));
        
        // Read the response
        boost::asio::streambuf response_buffer;
        boost::asio::read_until(socket, response_buffer, '\n');
        
        istream response_stream(&response_buffer);
        string response;
        getline(response_stream, response);
        cout << "Server Response: " << response << endl;
    }

private:
    boost::asio::io_service io_service;
    tcp::socket socket;
};