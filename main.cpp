#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <boost/config.hpp>
#include "json.hpp"
#include <fstream>
#include <thread>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;
using json = nlohmann::json;

int response_delay_ms = 0;

std::string timestamp()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&now_c);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

template <class Body, class Allocator, class Send>
void handle_request(http::request<Body, http::basic_fields<Allocator>> &&req, Send &&send)
{
    if (req.method() == http::verb::get && req.target() == "/hello")
    {
        std::cout << "[" << timestamp() << "] 📥 Received GET /hello\n";
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

        std::cout << "[" << timestamp() << "] ✅ Responded with 200 OK\n";
        return send(std::move(res));
    }

    if (req.method() == http::verb::post && req.target() == "/config")
    {
        std::cout << "[" << timestamp() << "] 📥 Received POST /config\n";
        try
        {
            auto config_json = json::parse(req.body());

            if (config_json.contains("response_delay_ms"))
            {
                response_delay_ms = config_json["response_delay_ms"].template get<int>();
                std::cout << "[" << timestamp() << "] 🔧 Set response_delay_ms to " << response_delay_ms << " ms\n";
            }

            http::string_body::value_type body = "Config updated";
            http::response<http::string_body> res{
                std::piecewise_construct,
                std::make_tuple(std::move(body)),
                std::make_tuple(http::status::ok, req.version())};
            res.set(http::field::content_type, "text/plain");
            res.keep_alive(req.keep_alive());
            res.prepare_payload();
            return send(std::move(res));
        }
        catch (const std::exception &e)
        {
            http::response<http::string_body> res{http::status::bad_request, req.version()};
            res.set(http::field::content_type, "text/plain");
            res.body() = std::string("Invalid JSON: ") + e.what();
            res.prepare_payload();
            return send(std::move(res));
        }
    }

    std::cout << "[" << timestamp() << "] ❌ Unknown path: " << req.target() << "\n";
    http::response<http::string_body> res{http::status::not_found, req.version()};
    res.set(http::field::content_type, "text/plain");
    res.body() = "Not found";
    res.prepare_payload();
    return send(std::move(res));
}

void do_session(tcp::socket socket)
{
    boost::beast::error_code ec;
    boost::beast::flat_buffer buffer;

    http::request<http::string_body> req;
    http::read(socket, buffer, req, ec);

    if (ec)
    {
        std::cerr << "[" << timestamp() << "] ⚠️  Error reading request: " << ec.message() << "\n";
        return;
    }

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
        std::thread([s = std::move(socket)]() mutable {
            do_session(std::move(s));
        }).detach();
    }
}

int main()
{
    try
    {
        boost::asio::io_context ioc{1};
        std::cout << "[" << timestamp() << "] 🚀 Server started at http://0.0.0.0:9798\n";
        server(ioc, 9798);
    }
    catch (const std::exception &e)
    {
        std::cerr << "[" << timestamp() << "] ❗ Fatal error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
