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
    auto http_config = cfg["modules"]["http"].AsDict();
    if (http_config.find("enableTls") != http_config.end()) {
        enable_tls_ = http_config["enableTls"]->AsBool();
    }
}

void HttpModule::Run(ziapi::http::IRequestOutputQueue &requests, ziapi::http::IResponseInputQueue &responses)
{
    if (enable_tls_) {
        tls_server_.emplace(requests, responses);
        server_->Start(num_threads_);
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
