#pragma once

#include <array>
#include <asio.hpp>
#include <atomic>
#include <memory>
#include <string>
#include <ziapi/Http.hpp>
#include <ziapi/Logger.hpp>

#include "ConnectionManager.hpp"
#include "RequestStreamParser.hpp"
#include "SafeRequestQueue.hpp"

class Connection : public std::enable_shared_from_this<Connection> {
public:
    Connection(asio::io_context &ctx, asio::ip::tcp::socket socket, SafeRequestQueue &requests,
               ConnectionManager<Connection> &conn_manager);

    void Start();

    void Close();

    bool IsOpen();

    void Send(const ziapi::http::Response &res);

    const asio::ip::tcp::endpoint &RemoteEndpoint() const;

    void ShouldClose(bool should_close);

private:
    void DoRead();

    void DoWrite();

    template <typename CompletionHandler>
    void CallbackWrapper(std::error_code ec, CompletionHandler handler)
    {
        if (ec) {
            if (IsOpen()) {
                conn_manager_.Close(shared_from_this());
            }
            return;
        }
        handler();
    }

    asio::io_context::strand strand_;

    asio::ip::tcp::socket socket_;

    SafeRequestQueue &requests_;

    ConnectionManager<Connection> &conn_manager_;

    std::array<char, 4096> buffer_;

    asio::ip::tcp::endpoint remote_endpoint_;

    std::string outbuf_;

    bool should_close_;

    std::atomic_bool is_open_;

    RequestStreamParser parser_stream_;
};
