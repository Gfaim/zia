#pragma once

#include <asio.hpp>
#include <atomic>
#include <condition_variable>
#include <iostream>
#include <ziapi/Module.hpp>

class HttpModule : public ziapi::INetworkModule {
public:
    HttpModule(unsigned int num_threads) : num_threads_(num_threads), acceptor_(ctx_) {}

    void Init(const ziapi::config::Node &) override {}

    [[nodiscard]] ziapi::Version GetVersion() const noexcept override { return ziapi::Version(1, 0, 0); }

    [[nodiscard]] ziapi::Version GetCompatibleApiVersion() const noexcept override { return ziapi::Version(3, 0, 0); }

    [[nodiscard]] const char *GetName() const noexcept override { return kModuleName; }

    [[nodiscard]] const char *GetDescription() const noexcept override { return kModuleDescription; }

    void Run(ziapi::http::IRequestOutputQueue &requests, ziapi::http::IResponseInputQueue &responses) override;

    void Terminate() override;

private:
    void StartThreadPool();

    void AsyncAccept();

    static const char *kModuleDescription;

    static const char *kModuleName;

    asio::io_context ctx_;

    asio::ip::tcp::acceptor acceptor_;

    std::vector<std::pair<asio::ip::tcp::socket, asio::io_context::strand>> clients_;

    unsigned int num_threads_;

    std::vector<std::thread> thread_pool_;
};
