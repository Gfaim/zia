#include "Http.hpp"

#include <asio.hpp>
#include <iostream>
#include <thread>

#include "dylib/dylib.hpp"
#include "ziapi/Logger.hpp"

const char *HttpModule::kModuleName = "HTTP";

const char *HttpModule::kModuleDescription = "TCP network layer and HTTP parser";

HttpModule::HttpModule(unsigned int num_threads) : num_threads_(num_threads), enable_tls_(false) {}

void HttpModule::Init(const ziapi::config::Node &cfg)
{
    const auto &modules_config = cfg["modules"].AsDict();
    const auto &http_config_it = modules_config.find("http");

    if (http_config_it != modules_config.end()) {
        const auto &http_config = http_config_it->second->AsDict();
        const auto &ssl_config_it = http_config.find("ssl");
        if (ssl_config_it != http_config.end()) {
            const auto &ssl_config = ssl_config_it->second->AsDict();
            enable_tls_ = ssl_config.at("enable")->AsBool();
            if (enable_tls_) {
                cert_ = ssl_config.at("certificatePath")->AsString();
                key_ = ssl_config.at("keyPath")->AsString();
            }
        }
    }
}

void HttpModule::Run(ziapi::http::IRequestOutputQueue &requests, ziapi::http::IResponseInputQueue &responses)
{
    if (enable_tls_) {
        ziapi::Logger::Info("http: tls enabled (cert=", cert_, "key=", key_, ")");
        tls_server_.emplace(requests, responses, cert_, key_);
        tls_server_->Start(num_threads_);
    } else {
        server_.emplace(requests, responses);
        server_->Start(num_threads_);
    }
}

void HttpModule::Terminate()
{
    if (enable_tls_) {
        tls_server_->Stop();
    } else {
        server_->Stop();
    }
}

DYLIB_API ziapi::IModule *LoadZiaModule() { return new HttpModule(10); }
