#include "http/Module.hpp"

#include <asio.hpp>
#include <iostream>
#include <thread>

#include "dylib/dylib.hpp"
#include "ziapi/Logger.hpp"

const char *HttpModule::kModuleName = "HTTP";

const char *HttpModule::kModuleDescription = "TCP network layer and HTTP parser";

HttpModule::HttpModule(unsigned int num_threads)
    : num_threads_(num_threads), strand_(ctx_), acceptor_(ctx_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 80))
{
}

void HttpModule::Init(const ziapi::config::Node &) {}

void HttpModule::Run(ziapi::http::IRequestOutputQueue &, ziapi::http::IResponseInputQueue &)
{
    AsyncAccept();
    StartThreadPool();
}

void HttpModule::AsyncAccept()
{
    acceptor_.async_accept(asio::bind_executor(strand_, [&](const auto &ec, asio::ip::tcp::socket socket) {
        if (ec) {
            return;
        }
        ziapi::Logger::Info("New connection established with ", socket.remote_endpoint());
        OnConnection(std::move(socket));
        AsyncAccept();
    }));
}

void HttpModule::OnConnection(asio::ip::tcp::socket socket)
{
    auto &conn = clients_.emplace_back(asio::io_context::strand(ctx_), std::move(socket));
}

void HttpModule::StartAsyncReadLoop(Connection &conn) {}

void HttpModule::StartThreadPool()
{
    for (size_t i = 0; i < num_threads_; i++) {
        thread_pool_.emplace_back([&]() { ctx_.run(); });
    }
}

void HttpModule::Terminate() {}

DYLIB_API ziapi::IModule *LoadZiaModule() { return new HttpModule(5); }
