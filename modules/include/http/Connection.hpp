#pragma once

#include <asio.hpp>
#include <string>

#include "http/AsyncOp.hpp"
#include "http/SafeRequestQueue.hpp"
#include "ziapi/Http.hpp"

class Connection {
public:
    Connection(asio::io_context::strand strand, asio::ip::tcp::socket socket, SafeRequestQueue &requests);

    void AsyncSend(const ziapi::http::Response &res, std::function<void(std::error_code)> completion_handler);

    void AsyncRead(std::function<void(std::error_code)> completion_handler);

    void Close();

private:
    SafeRequestQueue &requests_;

    ReadRequestOp readop_;

    WriteResponseOp writeop_;

    asio::io_context::strand strand_;

    asio::ip::tcp::socket socket_;
};
