#include "http/Module.hpp"

#include <asio.hpp>
#include <iostream>
#include <thread>

#include "dylib/dylib.hpp"
#include "ziapi/Logger.hpp"

const char *HttpModule::kModuleName = "HTTP";

const char *HttpModule::kModuleDescription = "TCP network layer and HTTP parser";

void HttpModule::Run(ziapi::http::IRequestOutputQueue &requests, ziapi::http::IResponseInputQueue &responses)
{
    asio::ip::tcp::acceptor acceptor(ctx_);

    AsyncAccept();
    StartThreadPool();
}

void HttpModule::AsyncAccept()
{
    acceptor_.async_accept([&](const auto &ec, auto socket) {
        if (ec) {
        }
        ziapi::Logger::Info("New connection established");
        clients_.emplace_back(std::make_pair(std::move(socket), asio::make_strand(ctx_)));
    });
}

void HttpModule::StartThreadPool()
{
    for (size_t i = 0; i < num_threads_; i++) {
        thread_pool_.emplace_back([&]() { ctx_.run(); });
    }
}

void HttpModule::Terminate() {}

DYLIB_API ziapi::IModule *LoadZiaModule() { return new HttpModule(5); }
