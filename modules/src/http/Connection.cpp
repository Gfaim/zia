#include "http/Connection.hpp"

#include <asio.hpp>

#include "ziapi/Logger.hpp"

Connection::Connection(asio::io_context::strand strand, asio::ip::tcp::socket socket, SafeRequestQueue &requests)
    : requests_(requests), readop_(), writeop_(), strand_(std::move(strand)), socket_(std::move(socket))
{
}

void Connection::AsyncRead(std::function<void(std::error_code)> completion_handler) {}

void Connection::Close() { socket_.close(); }

void Connection::AsyncSend(const ziapi::http::Response &, std::function<void(std::error_code)> completion_handler) {}
