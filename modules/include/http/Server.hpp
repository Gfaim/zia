#pragma once

#include <asio.hpp>
#include <ziapi/Http.hpp>

#include "http/ConnectionManager.hpp"
#include "http/SafeRequestQueue.hpp"

class Server {
public:
    Server(ziapi::http::IRequestOutputQueue &requests, ziapi::http::IResponseInputQueue &responses);

    void Start(unsigned int num_threads);

    void Stop();

private:
    void AcceptConnections();

    void DoAccept();

    void StartThreadPool(unsigned int num_threads);

    asio::io_context ctx_;

    asio::io_context::strand strand_;

    asio::ip::tcp::acceptor acceptor_;

    ConnectionManager conn_manager_;

    std::vector<std::thread> thread_pool_;

    SafeRequestQueue requests_;

    ziapi::http::IResponseInputQueue &responses_;
};
