#include <iostream>
#include <boost/asio.hpp>
#include <windows.h>

namespace asio = boost::asio;
using boost::asio::ip::tcp;
using boost::asio::ip::address;

const uint8_t on_connection_message = 1;

int main() {
    setlocale(LC_ALL, "russian");
    boost::system::error_code ec;
    asio::io_context context;
    tcp::endpoint remote_host(address::from_string("192.168.1.100"), 6000);
    for (;;) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        tcp::socket client(context);
        client.connect(remote_host, ec);
        client.write_some(asio::buffer(&on_connection_message, 1), ec);
        if (ec) {
            ec.clear();
            client.close();
            continue;
        }
        for (;;) {
            INPUT input;
            client.read_some(asio::buffer(&input, sizeof(input)), ec);
            if (ec) {
                ec.clear();
                client.close();
                break;
            }
            // SendInput(1, &input, sizeof(INPUT));
            std::cout << input.type << std::endl;
        }
    }
}