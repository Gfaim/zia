#include "logger/Module.hpp"

#include "dylib/dylib.hpp"

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

void LoggerModule::PostProcess(ziapi::http::Context &ctx, ziapi::http::Response &res)
{
    std::stringstream ss;

    // Exemple: ` [X] 404: Not found (GET /test, 2s)`
    ss << std::to_string((int)res.status_code) << ": " << res.reason << " ("
       << std::any_cast<std::string>(ctx["method"]) << " " << std::any_cast<std::string>(ctx["target"]) << ", "
       << std::setprecision(2) << difftime(std::time(nullptr), std::any_cast<time_t>(ctx["timestamp"])) << "s)";
    if (!res.body.empty()) {
        ss << std::endl << "BODY:\n\"" << res.body << "\"";
    }
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

DYLIB_API ziapi::IModule *LoadZiaModule() { return new LoggerModule(); }
