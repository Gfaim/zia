#pragma once

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <ziapi/Module.hpp>

class HttpModule : public ziapi::INetworkModule {
public:
    HttpModule() : must_stop_{false}, has_stopped_{} {}

    void Init(const ziapi::config::Node &) override {}

    [[nodiscard]] ziapi::Version GetVersion() const noexcept override { return ziapi::Version(1, 0, 0); }

    [[nodiscard]] ziapi::Version GetCompatibleApiVersion() const noexcept override { return ziapi::Version(3, 0, 0); }

    [[nodiscard]] const char *GetName() const noexcept override { return kModuleName; }

    [[nodiscard]] const char *GetDescription() const noexcept override { return kModuleDescription; }

    void Run(ziapi::http::IRequestOutputQueue &requests, ziapi::http::IResponseInputQueue &responses) override;

    void Terminate() override;

private:
    static const char *kModuleDescription;

    static const char *kModuleName;

    std::atomic<bool> must_stop_;

    std::condition_variable has_stopped_;
};
