#pragma once

#include <asio.hpp>
#include <string>

class Connection {
public:
    Connection(asio::io_context::strand strand, asio::ip::tcp::socket socket);

    void AsyncRead(std::function<void(std::error_code, std::string)> &&completion_handler);

    void AsyncWrite(const asio::const_buffer &buf, std::function<void(std::error_code, std::size_t)> &&completion_handler);

    void Close();

private:
    asio::io_context::strand strand_;

    asio::ip::tcp::socket socket_;

    std::string writebuf_;

    std::string readbuf_;
};
