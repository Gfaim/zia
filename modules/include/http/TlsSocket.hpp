#include <asio.hpp>
#include <asio/ssl.hpp>

class TlsSocket : public asio::ssl::stream<asio::ip::tcp::socket> {
public:
    using Base = asio::ssl::stream<asio::ip::tcp::socket>;

    using Base::Base;

    TlsSocket(Base &&other) : Base(std::move(other)) {}

    auto remote_endpoint() { return next_layer().remote_endpoint(); }

    auto is_open() { return next_layer().is_open(); }

    auto close() { return next_layer().close(); }
};
