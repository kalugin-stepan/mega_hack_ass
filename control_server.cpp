#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <atomic>

namespace asio = boost::asio;
using boost::asio::ip::tcp;

void handle_reciver(tcp::socket client);
void handle_sender(tcp::socket client);

std::atomic<bool> data_ready;
char data[40];

int main() {
    setlocale(LC_ALL, "russian");
    data_ready.exchange(false);
    boost::system::error_code ec;
    asio::io_context context;
    tcp::acceptor acceptor(context, tcp::endpoint(tcp::v4(), 6000));
    for (;;) {
        tcp::socket client = acceptor.accept(ec);
        if (ec) {
            std::cerr << ec.message() << std::endl;
            continue;
        }
        uint8_t who;
        client.read_some(asio::buffer(&who, 1), ec);
        if (ec) {
            std::cerr << ec << std::endl;
            continue;
        }
        if (who == 1) {
            std::cout << "Reciver " << client.remote_endpoint().address().to_string() << ":" << client.remote_endpoint().port() << " connected" << std::endl;
            std::thread(handle_reciver, std::move(client)).detach();
            continue;
        }
        if (who == 2) {
            std::cout << "Sender " << client.remote_endpoint().address().to_string() << ":" << client.remote_endpoint().port() << " connected" << std::endl;
            std::thread(handle_sender, std::move(client)).detach();
        }
    }
}

void handle_reciver(tcp::socket client) {
    boost::system::error_code ec;
    for (;;) {
        if (!data_ready.load()) continue;
        client.write_some(asio::buffer(data, 40), ec);
        if (ec) {
            std::cerr << ec.message() << std::endl;
            std::cout << "Reciver " << client.remote_endpoint().address().to_string() << ":" << client.remote_endpoint().port() << " disconnected" << std::endl;
            break;
        }
        data_ready.exchange(false);
    }
}

void handle_sender(tcp::socket client) {
    boost::system::error_code ec;
    client.set_option(boost::asio::detail::socket_option::integer<SOL_SOCKET, SO_RCVTIMEO>{ 60000 });
    for (;;) {
        if (data_ready.load()) continue;
        client.read_some(asio::buffer(data, 40), ec);
        if (ec) {
            std::cerr << ec.message() << std::endl;
            std::cout << "Sender " << client.remote_endpoint().address().to_string() << ":" << client.remote_endpoint().port() << " disconnected" << std::endl;
            break;
        }
        data_ready.exchange(true);
    }
}