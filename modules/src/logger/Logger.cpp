#include "Logger.hpp"

#include "dylib/dylib.hpp"

void LoggerModule::Init(const ziapi::config::Node &cfg)
{
    logs_route_ = cfg["modules"]["logger"]["route"].AsString();
    requests_buffer_size_ = cfg["modules"]["logger"]["buffer_size"].AsInt();
}

[[nodiscard]] double LoggerModule::GetPostProcessorPriority() const noexcept { return 1; }

[[nodiscard]] bool LoggerModule::ShouldPostProcess(const ziapi::http::Context &, const ziapi::http::Request &,
                                                   const ziapi::http::Response &) const
{
    return true;
}

[[nodiscard]] double LoggerModule::GetPreProcessorPriority() const noexcept { return 0; }

[[nodiscard]] bool LoggerModule::ShouldPreProcess(const ziapi::http::Context &, const ziapi::http::Request &) const
{
    return true;
}

void LoggerModule::PostProcess(ziapi::http::Context &ctx, const ziapi::http::Request &req, ziapi::http::Response &res)
{
    auto timestamptz = difftime(std::time(nullptr), std::any_cast<time_t>(ctx["timestamp"]));
    requests_.push_back(RequestInfos{timestamptz, req, res});
    while (requests_.size() >= requests_buffer_size_) {
        requests_.erase(requests_.begin());
    }
    std::stringstream ss;

    // Exemple: ` [X] 404: Not found (GET /test, 2s)`
    ss << std::to_string((int)res.status_code) << ": " << res.reason << " ("
       << std::any_cast<std::string>(ctx["method"]) << " " << std::any_cast<std::string>(ctx["target"]) << ", "
       << std::setprecision(2) << timestamptz << "s)";
    if ((int)res.status_code < 300) {
        ziapi::Logger::Info(ss.str());
    } else if ((int)res.status_code < 400) {
        ziapi::Logger::Warning(ss.str());
    } else {
        ziapi::Logger::Error(ss.str());
    }
}

void LoggerModule::PreProcess(ziapi::http::Context &ctx, ziapi::http::Request &req)
{
    ctx["timestamp"] = std::time(nullptr);
    ctx["target"] = req.target;
    ctx["method"] = req.method;
}

void LoggerModule::Handle(ziapi::http::Context &ctx, const ziapi::http::Request &req, ziapi::http::Response &res) {}

[[nodiscard]] double LoggerModule::GetHandlerPriority() const noexcept { return 0; }

[[nodiscard]] bool LoggerModule::ShouldHandle(const ziapi::http::Context &ctx, const ziapi::http::Request &req) const
{
    return req.method == "GET" && req.target == logs_route_;
}

DYLIB_API ziapi::IModule *LoadZiaModule() { return new LoggerModule(); }
