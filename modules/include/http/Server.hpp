#pragma once

#include <asio.hpp>
#include <condition_variable>
#include <ziapi/Http.hpp>

#include "Connection.hpp"
#include "ConnectionManager.hpp"
#include "SafeRequestQueue.hpp"

class Server {
public:
    Server(ziapi::http::IRequestOutputQueue &requests, ziapi::http::IResponseInputQueue &responses)
        : ctx_(), strand_(ctx_), acceptor_(ctx_), conn_manager_(), requests_(requests), responses_(responses)
    {
    }

    void Start(unsigned int num_threads_)
    {
        AcceptConnections();
        StartThreadPool(num_threads_);
        while (true) {
            if (responses_.Size()) {
                auto res = responses_.Pop();
                if (res) {
                    conn_manager_.Dispatch(*res);
                }
            }
            std::this_thread::sleep_for(std::chrono::nanoseconds(1000));
        }
    }

    void Stop()
    {
        ctx_.stop();
        for (auto &t : thread_pool_) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

private:
    void AcceptConnections()
    {
        acceptor_.open(asio::ip::tcp::v4());
        acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
        acceptor_.bind(asio::ip::tcp::endpoint(asio::ip::address_v4::any(), 80));
        acceptor_.listen();
        DoAccept();
    }

    void DoAccept()
    {
        acceptor_.async_accept([this](auto ec, auto socket) {
            if (!acceptor_.is_open()) {
                return;
            }
            if (!ec) {
                conn_manager_.Add(std::make_shared<Connection>(ctx_, std::move(socket), requests_, conn_manager_));
            }
            DoAccept();
        });
    }

    void StartThreadPool(unsigned int num_threads)
    {
        ziapi::Logger::Info("starting network thread pool (size ", num_threads, ")");
        for (size_t i = 0; i < num_threads; i++) {
            thread_pool_.emplace_back([&]() { ctx_.run(); });
        }
    }

    asio::io_context ctx_;

    asio::io_context::strand strand_;

    asio::ip::tcp::acceptor acceptor_;

    ConnectionManager<Connection> conn_manager_;

    std::vector<std::thread> thread_pool_;

    std::condition_variable must_stop_cv_;

    SafeRequestQueue requests_;

    ziapi::http::IResponseInputQueue &responses_;
};

class TlsServer {
public:
    TlsServer(ziapi::http::IRequestOutputQueue &requests, ziapi::http::IResponseInputQueue &responses)
        : ctx_(),
          strand_(ctx_),
          acceptor_(ctx_),
          ssl_context_(asio::ssl::context_base::tls),
          conn_manager_(),
          requests_(requests),
          responses_(responses)
    {
    }

    void Start(unsigned int num_threads_)
    {
        AcceptConnections();
        StartThreadPool(num_threads_);
        while (true) {
            if (responses_.Size()) {
                auto res = responses_.Pop();
                if (res) {
                    conn_manager_.Dispatch(*res);
                }
            }
            std::this_thread::sleep_for(std::chrono::nanoseconds(1000));
        }
    }

    void Stop()
    {
        ctx_.stop();
        for (auto &t : thread_pool_) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

private:
    void AcceptConnections()
    {
        acceptor_.open(asio::ip::tcp::v4());
        acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
        acceptor_.bind(asio::ip::tcp::endpoint(asio::ip::address_v4::any(), 443));
        acceptor_.listen();
        DoAccept();
    }

    void DoAccept()
    {
        auto socket = std::make_shared<asio::ssl::stream<asio::ip::tcp::socket>>(ctx_, ssl_context_);
        acceptor_.async_accept(socket->next_layer(), [this, socket](auto ec) {
            if (!acceptor_.is_open()) {
                return;
            }
            if (!ec) {
                asio::ssl::stream<asio::ip::tcp::socket> sock(std::move(*socket));
                conn_manager_.Add(std::make_shared<TlsConnection>(ctx_, std::move(sock), requests_, conn_manager_));
            }
            DoAccept();
        });
    }

    void StartThreadPool(unsigned int num_threads)
    {
        ziapi::Logger::Info("starting network thread pool (size ", num_threads, ")");
        for (size_t i = 0; i < num_threads; i++) {
            thread_pool_.emplace_back([&]() { ctx_.run(); });
        }
    }

    asio::io_context ctx_;

    asio::io_context::strand strand_;

    asio::ip::tcp::acceptor acceptor_;

    asio::ssl::context ssl_context_;

    ConnectionManager<TlsConnection> conn_manager_;

    std::vector<std::thread> thread_pool_;

    std::condition_variable must_stop_cv_;

    SafeRequestQueue requests_;

    ziapi::http::IResponseInputQueue &responses_;
};
