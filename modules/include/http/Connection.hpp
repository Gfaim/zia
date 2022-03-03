#pragma once

#include <array>
#include <asio.hpp>
#include <atomic>
#include <memory>
#include <string>
#include <ziapi/Logger.hpp>

#include "http/RequestStreamParser.hpp"
#include "http/SafeRequestQueue.hpp"
#include "ziapi/Http.hpp"

class ConnectionManager;

class Connection : public std::enable_shared_from_this<Connection> {
public:
    Connection(asio::io_context &ctx, asio::ip::tcp::socket socket, SafeRequestQueue &requests,
               ConnectionManager &conn_manager);

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
    void CallbackWrapper(std::error_code ec, CompletionHandler handler);

    asio::io_context::strand strand_;

    asio::ip::tcp::socket socket_;

    SafeRequestQueue &requests_;

    ConnectionManager &conn_manager_;

    std::array<char, 4096> buffer_;

    asio::ip::tcp::endpoint remote_endpoint_;

    std::string outbuf_;

    bool should_close_;

    std::atomic_bool is_open_;

    RequestStreamParser parser_stream_;
};

#include <memory>
#include <utility>

class ConnectionManager {
public:
    using SharedConnection = std::shared_ptr<Connection>;

    void Add(SharedConnection conn);

    void Close(SharedConnection conn);

    void CloseAll();

    void Dispatch(std::pair<ziapi::http::Response, ziapi::http::Context> &res);

private:
    std::mutex mu_;

    std::vector<SharedConnection> connections_;
};

template <typename CompletionHandler>
void Connection::CallbackWrapper(std::error_code ec, CompletionHandler handler)
{
    if (ec) {
        if (IsOpen()) {
            conn_manager_.Close(shared_from_this());
        }
        return;
    }
    handler();
}
