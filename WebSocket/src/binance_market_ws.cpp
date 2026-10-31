#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace ssl = asio::ssl;
using tcp = asio::ip::tcp;

int main() {
    constexpr char host[] = "data-stream.binance.vision";
    constexpr char port[] = "443";
    constexpr char path[] = "/ws/btcusdt@aggTrade";
    constexpr int messages_to_read = 5;

    try {
        asio::io_context ioc;
        ssl::context tls_context{ssl::context::tls_client};
        tls_context.set_default_verify_paths();

        tcp::resolver resolver{ioc};
        websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws{ioc, tls_context};

        // SNI lets Binance choose the correct TLS certificate for this hostname.
        if (!SSL_set_tlsext_host_name(ws.next_layer().native_handle(), host)) {
            beast::error_code ec{
                static_cast<int>(::ERR_get_error()), asio::error::get_ssl_category()};
            throw beast::system_error{ec};
        }

        ws.next_layer().set_verify_mode(ssl::verify_peer);
        ws.next_layer().set_verify_callback(ssl::host_name_verification(host));

        const auto endpoints = resolver.resolve(host, port);
        beast::get_lowest_layer(ws).connect(endpoints);
        ws.next_layer().handshake(ssl::stream_base::client);

        ws.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));
        ws.handshake(host, path);

        std::cout << "Connected and subscribed to BTCUSDT aggregate trades.\n";
        for (int i = 0; i < messages_to_read; ++i) {
            beast::flat_buffer buffer;
            ws.read(buffer);
            std::cout << "Message " << (i + 1) << ": "
                      << beast::buffers_to_string(buffer.data()) << '\n';
        }

        ws.close(websocket::close_code::normal);
        std::cout << "Closed normally.\n";
    } catch (const std::exception& e) {
        std::cerr << "Binance WebSocket error: " << e.what() << '\n';
        return EXIT_FAILURE;
    }
}
