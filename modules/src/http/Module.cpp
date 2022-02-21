#include "http/Module.hpp"

#include <iostream>
#include <thread>

#include "dylib/dylib.hpp"

const char *HttpModule::kModuleName = "HTTP";

const char *HttpModule::kModuleDescription = "TCP network layer and HTTP parser";

void HttpModule::Run(ziapi::http::IRequestOutputQueue &requests, ziapi::http::IResponseInputQueue &responses)
{
    auto now_plus_some_time = std::chrono::system_clock::now() + std::chrono::seconds(3);

    while (true) {
        if (now_plus_some_time < std::chrono::system_clock::now()) {
            ziapi::http::Request req{};
            ziapi::http::Context ctx{};
            requests.Push(std::make_pair(req, ctx));
            std::cout << "BIM une requête" << std::endl;
            now_plus_some_time = std::chrono::system_clock::now() + std::chrono::seconds(1);
        }
        if (responses.Size()) {
            auto res_opt = responses.Pop();
            if (res_opt) {
                auto res = res_opt->first;
                std::cout << "<Response code=" << (int)res.status_code << ", body_size=" << res.body.size() << ">"
                          << std::endl;
            }
        }
        if (must_stop_) {
            /// We set must_stop_ to false to counteract spurious wakeups inside Terminate().
            must_stop_ = false;
            has_stopped_.notify_all();
            break;
        }
        std::this_thread::sleep_for(std::chrono::nanoseconds(1000));
    }
}

void HttpModule::Terminate()
{
    std::mutex mu;
    std::unique_lock<std::mutex> ul{mu};
    must_stop_ = true;
    /// Once the worker thread has gracefully shut down, it will set must_stop_ back to false.
    /// We wait on the condition variable in a loop to counteract spurious wakeups.
    while (must_stop_) {
        has_stopped_.wait(ul);
    }
}

DYLIB_API ziapi::IModule *LoadZiaModule() { return new HttpModule(); }