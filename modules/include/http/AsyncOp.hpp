#pragma once

#include <asio.hpp>

#include "ziapi/Http.hpp"
#include "ziapi/Logger.hpp"

class ReadRequestOp {
public:
    ReadRequestOp() { ziapi::Logger::Info("read request op constructed"); }

    ReadRequestOp(ReadRequestOp &&) { ziapi::Logger::Info("read request op constructed"); }

    ReadRequestOp(const ReadRequestOp &) = delete;
    ReadRequestOp &operator=(const ReadRequestOp &) = delete;

    void operator()(std::error_code) { ziapi::Logger::Info("read request op completion handler invoked"); }
};

class WriteResponseOp {
public:
    void operator()(std::error_code) {}
};

// namespace asio::http {

// /// Composed asynchronous read operation on a TCP socket to fully read an HTTP request object.
// template <typename AsyncReadStream, typename CompletionHandler>
// void async_read_request(AsyncReadStream &s, ziapi::http::Request &req, CompletionHandler &&handler)
// {
// }

// /// Composed asynchronous write operation on a TCP socket to fully write an HTTP response object.
// template <typename AsyncWriteStream, typename CompletionHandler>
// void async_write_response(AsyncWriteStream &s, ziapi::http::Response &res, CompletionHandler &&handler)
// {
// }

// }  // namespace asio::http
