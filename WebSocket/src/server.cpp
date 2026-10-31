#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
using tcp = asio::ip::tcp;

int main(int argc, char* argv[]) {
    try {
        const unsigned short port = argc > 1
            ? static_cast<unsigned short>(std::stoi(argv[1]))
            : 9002;

        asio::io_context ioc{1};
        tcp::acceptor acceptor{ioc, {tcp::v4(), port}};
        std::cout << "Listening on 127.0.0.1:" << port << '\n';

        for (;;) {
            tcp::socket socket{ioc};
            acceptor.accept(socket);
            std::cout << "TCP connection accepted; waiting for WebSocket handshake...\n";

            websocket::stream<tcp::socket> ws{std::move(socket)};
            ws.accept();
            std::cout << "WebSocket handshake completed.\n";

            for (;;) {
                beast::flat_buffer buffer;
                beast::error_code ec;
                ws.read(buffer, ec);
                if (ec == websocket::error::closed) {
                    std::cout << "Client closed the WebSocket connection.\n";
                    break;
                }
                if (ec) {
                    std::cerr << "Read error: " << ec.message() << '\n';
                    break;
                }

                const std::string message = beast::buffers_to_string(buffer.data());
                std::cout << "Received: " << message << '\n';
                ws.text(ws.got_text());
                ws.write(buffer.data());
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << '\n';
        return EXIT_FAILURE;
    }
}
