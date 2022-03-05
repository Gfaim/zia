#include "ReverseProxy.hpp"

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <dylib/dylib.hpp>
#include <ziapi/Http.hpp>
#include <ziapi/Logger.hpp>
#include <ziapi/Module.hpp>

#include "ResponseStreamParser.hpp"

using ssl_socket = asio::ssl::stream<asio::ip::tcp::socket>;

#include <iomanip>
#include <sstream>

std::string to_string(const ziapi::http::Request &r)
{
    float version = static_cast<float>(r.version) / 10.f;
    std::string version_str;
    std::string res;
    std::stringstream version_stream;

    version_stream << std::fixed << std::setprecision(1) << version;
    version_str = version_stream.str();
    res = r.method + " " + r.target + " HTTP/" + version_str + "\r\n";
    for (const auto &[key, value] : r.headers) {
        res += key + ": " + value + "\r\n";
    }
    res += "\r\n";
    res += r.body;
    return res;
}

template <typename TSyncStream>
ziapi::http::Response DoRequest(TSyncStream &s, const ziapi::http::Request &req)
{
    ziapi::http::Response res;
    auto req_string = to_string(req);
    ResponseStreamParser res_parser;

    try {
        asio::write(s, asio::buffer(req_string));
        while (!res_parser.Done()) {
            std::array<char, 4096> buffer;
            auto bytes_read = s.read_some(asio::buffer(buffer));
            res_parser.Feed(buffer.data(), bytes_read);
        }
        res = res_parser.GetResponse();
    } catch (const std::exception &e) {
        ziapi::Logger::Error("reverse proxy: unable to write to socket: ", e.what());
        res.Bootstrap(ziapi::http::Code::kInternalServerError, ziapi::http::reason::kInternalServerError);
        return res;
    }
    return res;
}

void ReverseProxy::Handle(ziapi::http::Context &, const ziapi::http::Request &req, ziapi::http::Response &res)
{
    asio::io_context ctx;
    asio::ip::tcp::resolver resolver(ctx);
    auto endpoints = resolver.resolve(address_, std::to_string(port_));

    if (enable_tls_) {
        asio::ssl::context ssl_context(asio::ssl::context::tls);
        ssl_socket socket(ctx, ssl_context);
        asio::connect(socket.next_layer(), endpoints);
        socket.handshake(asio::ssl::stream_base::client);
        res = DoRequest(socket, req);
    } else {
        asio::ip::tcp::socket socket(ctx);
        asio::connect(socket, endpoints);
        res = DoRequest(socket, req);
    }
}

DYLIB_API ziapi::IModule *LoadZiaModule() { return new ReverseProxy(); }
