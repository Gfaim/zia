#include "Connection.hpp"

#include <any>
#include <asio.hpp>
#include <ziapi/Logger.hpp>

#include "ConnectionManager.hpp"
#include "ResponseToString.hpp"

Connection::Connection(asio::io_context &ctx, asio::ip::tcp::socket socket, SafeRequestQueue &requests,
                       ConnectionManager &conn_manager)
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

void Connection::Start() { DoRead(); }

void Connection::DoRead()
{
    socket_.async_read_some(
        asio::buffer(buffer_), asio::bind_executor(strand_, [this, me = shared_from_this()](auto ec, auto bytes_read) {
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
                    ctx.emplace("client.socket.address",
                                std::make_any<std::string>(remote_endpoint_.address().to_string()));
                    ctx.emplace("client.socket.port", std::make_any<std::uint16_t>(remote_endpoint_.port()));
                    if (req.headers.find(ziapi::http::header::kConnection) == req.headers.end()) {
                        ctx.emplace("http.connection", std::make_any<std::string>("close"));
                    } else {
                        ctx.emplace("http.connection",
                                    std::make_any<std::string>(req.headers[ziapi::http::header::kConnection]));
                    }
                    requests_.Push(std::make_pair(std::move(parser_stream_.GetRequest()), std::move(ctx)));
                    parser_stream_.Clear();
                }
                DoRead();
            });
        }));
}

void Connection::DoWrite()
{
    asio::async_write(socket_, asio::buffer(outbuf_),
                      asio::bind_executor(strand_, [this, me = shared_from_this()](auto ec, auto) {
                          CallbackWrapper(ec, [this, &me]() {
                              if (IsOpen() && should_close_) {
                                  conn_manager_.Close(me);
                              }
                          });
                      }));
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