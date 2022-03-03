#include "http/Connection.hpp"

#include <any>
#include <asio.hpp>
#include <ziapi/Logger.hpp>

#include "http/ConnectionManager.hpp"
#include "http/ResponseToString.hpp"

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
                // try {
                //     std::string req(buffer_.data(), bytes_read);
                //     std::cout << req;
                //     parser_stream_.Feed(buffer_.data(), bytes_read);
                // } catch (const std::exception &e) {
                //     ziapi::Logger::Debug("http error: ", e.what());
                //     if (IsOpen()) {
                //         conn_manager_.Close(me);
                //     }
                //     return;
                // }
                // if (parser_stream_.Done()) {
                ziapi::http::Context ctx;
                ziapi::http::Request req;

                req.target = "/src";
                req.method = ziapi::http::method::kGet;
                req.version = ziapi::http::Version::kV1_1;
                req.headers.emplace("Connection", "close");
                ctx.emplace("client.socket.address",
                            std::make_any<std::string>(remote_endpoint_.address().to_string()));
                ctx.emplace("client.socket.port", std::make_any<std::uint16_t>(remote_endpoint_.port()));
                if (req.headers.find("Connection") == req.headers.end()) {
                    ctx.emplace("http.connection", std::make_any<std::string>("close"));
                } else {
                    ctx.emplace("http.connection", std::make_any<std::string>(req.headers["end"]));
                }
                requests_.Push(std::make_pair(std::move(req), std::move(ctx)));

                // requests_.Push(std::make_pair(std::move(parser_stream_.GetRequest()), std::move(ctx)));
                // parser_stream_.Clear();
                // }
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
