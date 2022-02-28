#include "http/Connection.hpp"

#include <asio.hpp>

Connection::Connection(asio::io_context::strand strand, asio::ip::tcp::socket socket)
    : strand_(std::move(strand)), socket_(std::move(socket)), writebuf_(), readbuf_()
{
}

void Connection::AsyncRead(std::function<void(std::error_code, std::string)> &&completion_handler)
{
    socket_.async_wait(asio::ip::tcp::socket::wait_read, [&](auto ec) {
        if (ec) {
            completion_handler(ec, std::string());
            return;
        }
        readbuf_.clear();
        readbuf_.resize(socket_.available());
        asio::async_read(socket_, asio::buffer(readbuf_.data(), readbuf_.size()),
                         [completion_handler, this](auto ec, auto bytes_read) {
                             readbuf_.resize(bytes_read);
                             completion_handler(ec, std::move(readbuf_));
                         });
    });
}

void Connection::Close() { socket_.close(); }

void Connection::AsyncWrite(const asio::const_buffer &buf,
                            std::function<void(std::error_code, std::size_t)> &&completion_handler)
{
    writebuf_.assign((char *)buf.data(), buf.size());
    asio::async_write(socket_, asio::buffer(writebuf_.data(), writebuf_.size()),
                      [completion_handler](auto ec, auto bytes_written) { completion_handler(ec, bytes_written); });
}
