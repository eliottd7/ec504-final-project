#include <iostream>
#include <boost/asio.hpp>

class StorageServer {
public:
    StorageServer(int port) : acceptor(io_service, tcp::endpoint(tcp::v4(), port)) {
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

class Session : public std::enable_shared_from_this<Session> {
public:
    explicit Session(boost::asio::io_service &io_service) : socket(io_service) {}

    boost::asio::ip::tcp::socket &socket() { return socket; }

    void start() {
        boost::asio::async_read_until(socket, buffer, '\n',
                                      [this](boost::system::error_code ec, std::size_t) {
                                          if (!ec) handle_request();
                                      });
    }

    void handle_request() {
        // Parse the request from client and perform deduplication
    }

private:
    boost::asio::ip::tcp::socket socket;
    boost::asio::streambuf buffer;
};
