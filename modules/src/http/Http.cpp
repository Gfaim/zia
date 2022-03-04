#include "Http.hpp"

#include <asio.hpp>
#include <iostream>
#include <thread>

#include "dylib/dylib.hpp"
#include "ziapi/Logger.hpp"

const char *HttpModule::kModuleName = "HTTP";

const char *HttpModule::kModuleDescription = "TCP network layer and HTTP parser";

HttpModule::HttpModule(unsigned int num_threads) : num_threads_(num_threads) {}

void HttpModule::Init(const ziapi::config::Node &) {}

void HttpModule::Run(ziapi::http::IRequestOutputQueue &requests, ziapi::http::IResponseInputQueue &responses)
{
    server_.emplace(requests, responses);
    server_->Start(num_threads_);
}

void HttpModule::Terminate()
{
    if (server_) {
        server_->Stop();
    }
}

DYLIB_API ziapi::IModule *LoadZiaModule() { return new HttpModule(10); }
