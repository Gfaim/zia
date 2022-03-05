#include <asio.hpp>
#include <asio/ssl.hpp>

class Socket : public asio::ip::tcp::socket {
public:
    static constexpr std::uint16_t kProtocolPort{80};

    static constexpr const char *kProtocolName{"http"};

    using Base = asio::ip::tcp::socket;

    using Base::Base;

    Socket(Base &&other) : Base(std::move(other)) {}

    template <typename CompletionHandler>
    void Start(CompletionHandler &&handler)
    {
        handler(std::error_code());
    }
};

class TlsSocket : public asio::ssl::stream<asio::ip::tcp::socket> {
public:
    static constexpr std::uint16_t kProtocolPort{443};

    static constexpr const char *kProtocolName{"https"};

    using Base = asio::ssl::stream<asio::ip::tcp::socket>;

    using Base::Base;

    TlsSocket(Base &&other) : Base(std::move(other)) {}

    template <typename CompletionHandler>
    void Start(CompletionHandler &&handler)
    {
        this->async_handshake(asio::ssl::stream_base::server, [handler](auto ec) { handler(ec); });
    }

    auto remote_endpoint() { return next_layer().remote_endpoint(); }

    auto is_open() { return next_layer().is_open(); }

    auto close() { return next_layer().close(); }
};
