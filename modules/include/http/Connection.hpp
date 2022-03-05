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
#include "ResponseToString.hpp"
#include "SafeRequestQueue.hpp"
#include "Socket.hpp"

template <typename TSocket>
class BaseConnection : public std::enable_shared_from_this<BaseConnection<TSocket>> {
public:
    BaseConnection(asio::io_context &ctx, TSocket socket, SafeRequestQueue &requests,
                   ConnectionManager<BaseConnection<TSocket>> &conn_manager)
        : strand_(ctx),
          socket_(std::move(socket)),
          requests_(requests),
          conn_manager_(conn_manager),
          buffer_(),
          remote_endpoint_(),
          outbuf_(),
          should_close_(true),
          is_open_(true),
          parser_stream_{}
    {
        try {
            remote_endpoint_ = socket_.remote_endpoint();
        } catch (...) {
        }
    }

    void Start()
    {
        socket_.Start(
            [this, me = this->shared_from_this()](auto ec) { CallbackWrapper(ec, [this]() { this->DoRead(); }); });
    }

    void Close()
    {
        if (is_open_ && socket_.is_open()) {
            is_open_ = false;
            socket_.close();
        }
    }

    bool IsOpen()
    {
        if (is_open_ && socket_.is_open()) {
            return true;
        }
        is_open_ = false;
        return false;
    }

    void Send(const ziapi::http::Response &res)
    {
        outbuf_ = ResponseToString(res);
        DoWrite();
    }

    const asio::ip::tcp::endpoint &RemoteEndpoint() const { return remote_endpoint_; }

    void ShouldClose(bool should_close) { should_close_ = should_close; }

private:
    void DoRead()
    {
        socket_.async_read_some(
            asio::buffer(buffer_),
            asio::bind_executor(strand_, [this, me = this->shared_from_this()](auto ec, auto bytes_read) {
                CallbackWrapper(ec, [this, &bytes_read, me]() {
                    try {
                        parser_stream_.Feed(buffer_.data(), bytes_read);
                    } catch (const std::exception &e) {
                        if (IsOpen()) {
                            conn_manager_.Close(me);
                        }
                        return;
                    }
                    if (parser_stream_.Done()) {
                        ziapi::http::Context ctx;
                        auto req = parser_stream_.GetRequest();
                        PopulateRequestContext(req, ctx);
                        requests_.Push(std::make_pair(std::move(parser_stream_.GetRequest()), std::move(ctx)));
                        parser_stream_.Clear();
                    }
                    DoRead();
                });
            }));
    }

    void DoWrite()
    {
        asio::async_write(socket_, asio::buffer(outbuf_),
                          asio::bind_executor(strand_, [this, me = this->shared_from_this()](auto ec, auto) {
                              CallbackWrapper(ec, [this, &me]() {
                                  if (IsOpen() && should_close_) {
                                      conn_manager_.Close(me);
                                  }
                              });
                          }));
    }

    template <typename CompletionHandler>
    void CallbackWrapper(std::error_code ec, CompletionHandler handler)
    {
        if (ec) {
            if (IsOpen()) {
                conn_manager_.Close(this->shared_from_this());
            }
            return;
        }
        handler();
    }

    void PopulateRequestContext(ziapi::http::Request &req, ziapi::http::Context &ctx)
    {
        ctx.emplace("http.server.name", std::make_any<std::string>("Awougah"));
        ctx.emplace("http.server.port", std::make_any<std::uint16_t>(TSocket::kProtocolPort));
        ctx.emplace("http.server.protocol", std::make_any<std::string>(TSocket::kProtocolName));
        ctx.emplace("client.socket.address", std::make_any<std::string>(remote_endpoint_.address().to_string()));
        ctx.emplace("client.socket.port", std::make_any<std::uint16_t>(remote_endpoint_.port()));
        if (req.headers.find(ziapi::http::header::kConnection) == req.headers.end()) {
            ctx.emplace("http.connection", std::make_any<std::string>("close"));
        } else {
            ctx.emplace("http.connection", std::make_any<std::string>(req.headers[ziapi::http::header::kConnection]));
        }
    }

    asio::io_context::strand strand_;

    TSocket socket_;

    SafeRequestQueue &requests_;

    ConnectionManager<BaseConnection<TSocket>> &conn_manager_;

    std::array<char, 4096> buffer_;

    asio::ip::tcp::endpoint remote_endpoint_;

    std::string outbuf_;

    bool should_close_;

    std::atomic_bool is_open_;

    RequestStreamParser parser_stream_;
};

using Connection = BaseConnection<Socket>;

using TlsConnection = BaseConnection<TlsSocket>;
