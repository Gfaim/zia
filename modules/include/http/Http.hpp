#pragma once

#include <any>
#include <asio.hpp>
#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <iostream>
#include <optional>
#include <variant>
#include <ziapi/Module.hpp>

#include "Server.hpp"

class HttpModule : public ziapi::INetworkModule {
public:
    HttpModule(unsigned int num_threads);

    void Init(const ziapi::config::Node &) override;

    [[nodiscard]] ziapi::Version GetVersion() const noexcept override { return ziapi::Version(1, 0, 0); }

    [[nodiscard]] ziapi::Version GetCompatibleApiVersion() const noexcept override { return ziapi::Version(3, 0, 0); }

    [[nodiscard]] const char *GetName() const noexcept override { return kModuleName; }

    [[nodiscard]] const char *GetDescription() const noexcept override { return kModuleDescription; }

    void Run(ziapi::http::IRequestOutputQueue &requests, ziapi::http::IResponseInputQueue &responses) override;

    void Terminate() override;

private:
    static const char *kModuleDescription;

    static const char *kModuleName;

    unsigned int num_threads_;

    bool enable_tls_;

    std::filesystem::path cert_{};

    std::filesystem::path key_{};

    std::optional<Server> server_{};

    std::optional<TlsServer> tls_server_{};
};
