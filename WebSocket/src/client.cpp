#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
using tcp = asio::ip::tcp;

int main(int argc, char * argv[]) {
    try {
        const std::string host = argc > 1 ? argv[1] : "127.0.0.1";
        const std::string port = argc > 2 ? argv[2] : "9002";
        const std::string message = argc > 3 ? argv[3] : "hello websocket";

        asio::io_context ioc;
        tcp::resolver resolver{ioc};
        websocket::stream<tcp::socket> ws{ioc};
        const auto endpoints = resolver.resolve(host, port);
        asio::connect(ws.next_layer(), endpoints);

        ws.handshake(host + ":" + port, "/");
        std::cout << "WebSocket handshake completed.\n";

        ws.write(asio::buffer(message));
        beast::flat_buffer buffer;
        ws.read(buffer);
        std::cout << "Server echoed: " << beast::buffers_to_string(buffer.data()) << '\n';

        ws.close(websocket::close_code::normal);
        std::cout << "Close handshake completed.\n";
    } catch (const std::exception & e) {
        std::cerr << "Client error: " << e.what() << '\n';
        return EXIT_FAILURE;
    }
}