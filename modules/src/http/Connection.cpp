#include "http/Connection.hpp"

#include <any>
#include <asio.hpp>
#include <ziapi/Logger.hpp>

#include "http/ConnectionManager.hpp"
#include "http/ResponseToString.hpp"

Connection::Connection(asio::ip::tcp::socket socket, SafeRequestQueue &requests, ConnectionManager &conn_manager)
    : socket_(std::move(socket)),
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

void Connection::Start() { DoRead(); }

void Connection::DoRead()
{
    socket_.async_read_some(asio::buffer(buffer_), [this, me = shared_from_this()](auto ec, auto bytes_read) {
        CallbackWrapper(ec, [this, &bytes_read]() {
            parser_stream_.Feed(buffer_.data(), bytes_read);

            if (parser_stream_.Done()) {
                ziapi::http::Context ctx;
                ctx.emplace("client.socket.address",
                            std::make_any<std::string>(remote_endpoint_.address().to_string()));
                ctx.emplace("client.socket.port", std::make_any<std::uint16_t>(remote_endpoint_.port()));
                ctx.emplace("http.connection", std::make_any<std::string>("close"));

                requests_.Push(std::make_pair(std::move(parser_stream_.GetRequest()), std::move(ctx)));
                parser_stream_.Clear();
            }
            DoRead();
        });
    });
}

void Connection::DoWrite()
{
    asio::async_write(socket_, asio::buffer(outbuf_), [this, me = shared_from_this()](auto ec, auto) {
        CallbackWrapper(ec, [this, &me]() {
            if (IsOpen() && should_close_) {
                conn_manager_.Close(me);
            }
        });
    });
}

void Connection::Send(const ziapi::http::Response &r)
{
    outbuf_ = ResponseToString(r);
    DoWrite();
}

void Connection::ShouldClose(bool should_close) { should_close_ = should_close; }

bool Connection::IsOpen()
{
    if (is_open_ && socket_.is_open()) {
        return true;
    }
    is_open_ = false;
    return false;
}

const asio::ip::tcp::endpoint &Connection::RemoteEndpoint() const { return remote_endpoint_; }

void Connection::Close()
{
    if (is_open_ && socket_.is_open()) {
        is_open_ = false;
        socket_.close();
    }
}
