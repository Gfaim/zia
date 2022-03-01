#pragma once

#include <asio.hpp>
#include <memory>
#include <string>

#include "http/SafeRequestQueue.hpp"
#include "ziapi/Http.hpp"

class ConnectionManager;

class Connection : public std::enable_shared_from_this<Connection> {
public:
    Connection(asio::ip::tcp::socket socket, SafeRequestQueue &requests, ConnectionManager &conn_manager);

    void AsyncSend();

    void AsyncRead();

    void Close();

private:
    asio::ip::tcp::socket socket_;

    SafeRequestQueue &requests_;

    ConnectionManager &conn_manager_;
};
