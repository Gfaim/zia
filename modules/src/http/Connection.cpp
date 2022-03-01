#include "http/Connection.hpp"

#include <any>
#include <asio.hpp>
#include <ziapi/Logger.hpp>

Connection::Connection(asio::ip::tcp::socket socket, SafeRequestQueue &requests, ConnectionManager &conn_manager)
    : socket_(std::move(socket)), requests_(requests), conn_manager_(conn_manager)
{
}

// void Connection::AsyncRead(std::function<void(std::error_code)> handler)
// {
//     ziapi::Logger::Debug("conn.AsyncRead");
//     asio::http::async_read_request(socket_, reqbuf_, [=](auto ec) {
//         ziapi::Logger::Debug("async_read_request");
//         if (ec) {
//             handler(ec);
//             return;
//         }
//         ziapi::http::Context http_ctx;
//         http_ctx.emplace(std::string("client.socket.address"),
//                          std::make_any<std::string>(socket_.remote_endpoint().address().to_string()));
//         http_ctx.emplace(std::string("client.socket.port"),
//                          std::make_any<std::uint16_t>(socket_.remote_endpoint().port()));
//         requests_.Push(
//             std::make_pair<ziapi::http::Request, ziapi::http::Context>(std::move(reqbuf_), std::move(http_ctx)));
//         handler(ec);
//     });
// }

// void Connection::AsyncSend(const ziapi::http::Response &, std::function<void(std::error_code)> completion_handler) {}

void Connection::Close() { socket_.close(); }
