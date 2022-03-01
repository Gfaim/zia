#include "http/Server.hpp"

#include <ziapi/Logger.hpp>

Server::Server(ziapi::http::IRequestOutputQueue &requests, ziapi::http::IResponseInputQueue &responses)
    : ctx_(), strand_(ctx_), acceptor_(ctx_), conn_manager_(), requests_(requests), responses_(responses)
{
}

void Server::Start(unsigned int num_threads_)
{
    AcceptConnections();
    StartThreadPool(num_threads_);
}

void Server::AcceptConnections()
{
    acceptor_.open(asio::ip::tcp::v4());
    acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(asio::ip::tcp::endpoint(asio::ip::address_v4::any(), 80));
    acceptor_.listen();
    DoAccept();
}

void Server::DoAccept()
{
    acceptor_.async_accept([this](auto ec, auto socket) {
        if (!acceptor_.is_open()) {
            return;
        }
        if (!ec) {
            conn_manager_.Add(std::make_shared<Connection>(std::move(socket), requests_, conn_manager_));
        }
        DoAccept();
    });
}

void Server::StartThreadPool(unsigned int num_threads)
{
    ziapi::Logger::Info("starting network thread pool (size ", num_threads, ")");
    for (size_t i = 0; i < num_threads; i++) {
        thread_pool_.emplace_back([&]() { ctx_.run(); });
    }
}

void Server::Stop()
{
    ctx_.stop();
    for (auto &t : thread_pool_) {
        if (t.joinable()) {
            t.join();
        }
    }
}
