#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <boost/config.hpp>
#include <fstream>
#include <thread>
#include "json.hpp"
#include <chrono>
#include <iostream>

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;
using json = nlohmann::json;

int response_delay_ms = 0;

void load_config(const std::string &path)
{
    std::ifstream file(path);
    if (file)
    {
        json config;
        file >> config;
        response_delay_ms = config.value("response_delay_ms", 0);
        std::cout << "Response delay set to " << response_delay_ms << " ms\n";
    }
}

template <class Body, class Allocator, class Send>
void handle_request(http::request<Body, http::basic_fields<Allocator>> &&req, Send &&send)
{
    if (req.method() == http::verb::get && req.target() == "/hello")
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(response_delay_ms));
        http::string_body::value_type body = "Hello I got your message";
        auto const size = body.size();

        http::response<http::string_body> res{
            std::piecewise_construct,
            std::make_tuple(std::move(body)),
            std::make_tuple(http::status::ok, req.version())};
        res.set(http::field::server, "Boost.Beast");
        res.set(http::field::content_type, "text/plain");
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return send(std::move(res));
    }

    // 404 Not Found
    http::response<http::string_body> res{http::status::not_found, req.version()};
    res.set(http::field::content_type, "text/plain");
    res.body() = "Not found";
    res.prepare_payload();
    return send(std::move(res));
}

void do_session(tcp::socket socket)
{
    bool close = false;
    boost::beast::error_code ec;

    boost::beast::flat_buffer buffer;

    http::request<http::string_body> req;
    http::read(socket, buffer, req, ec);
    if (ec)
        return;

    auto const send = [&socket](auto &&response)
    {
        http::write(socket, response);
    };

    handle_request(std::move(req), send);
    socket.shutdown(tcp::socket::shutdown_send, ec);
}

void server(boost::asio::io_context &ioc, unsigned short port)
{
    tcp::acceptor acceptor(ioc, {tcp::v4(), port});
    for (;;)
    {
        tcp::socket socket(ioc);
        acceptor.accept(socket);
        std::thread([s = std::move(socket)]() mutable
                    { do_session(std::move(s)); })
            .detach();
    }
}

int main(int argc, char *argv[])
{
    load_config("config.json");

    try
    {
        boost::asio::io_context ioc{1};
        std::cout << "Server running on http://0.0.0.0:9798/hello\n";
        server(ioc, 9798);
    }
    catch (std::exception const &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
