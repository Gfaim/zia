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

    {
        std::scoped_lock lock(mu_);
        requests_.push_back(RequestInfos{timestamptz, req, res});
    
        while (requests_.size() >= requests_buffer_size_) {
            requests_.erase(requests_.begin());
        }
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

void LoggerModule::Handle(ziapi::http::Context &ctx, const ziapi::http::Request &req, ziapi::http::Response &res)
{
    res.status_code = ziapi::http::Code::kOK;
    res.body =
        "<!doctype html>\
<html>\
\
<head>\
    <meta charset=\"UTF-8\">\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
    <script src=\"https://cdn.tailwindcss.com\"></script>\
</head>\
\
<style>\
    * {\
        font-family: 'SF Pro Text', -apple-system, BlinkMacSystemFont, Roboto, 'Segoe UI', Helvetica, Arial, sans-serif, 'Apple Color Emoji', 'Segoe UI Emoji', 'Segoe UI Symbol';\
    }\
</style>\
\
<body class=\"flex items-center justify-center w-screen h-screen\">\
    <div class=\"w-[720px] h-[480px] bg-gray-100 rounded-xl shadow-xl border border-gray-200 relative overflow-hidden\">\
        <div class=\"flex p-2 h-8 items-center absolute\">\
            <div class=\"h-[14px] w-[14px] rounded-full bg-[#fe6055] border-[1px] border-gray-200 mr-1\"></div>\
            <div class=\"h-[14px] w-[14px] rounded-full bg-[#ffbc2d] border-[1px] border-gray-200 mr-1\"></div>\
            <div class=\"h-[14px] w-[14px] rounded-full bg-[#25c73e] border-[1px] border-gray-200\"></div>\
        </div>\
        <div class=\"flex justify-center p-1 h-8 font-semibold border-b border-gray-200 items-center\">\
            <img class=\"h-5 w-5 mx-2\"\
                src=\"https://media.macosicons.com/parse/files/macOSicons/af21153d07a2e92bde7b2ad155055489_low_res_1619092574091.png\">\
            Logs\
        </div>\
        <div class=\"bg-white h-full\">";
    for (auto it = requests_.rbegin(); it != requests_.rend(); ++it) {
        std::string color;
        if ((int)it->res.status_code < 300) {
            color = "#25c73e";
        } else if ((int)it->res.status_code < 400) {
            color = "#ffbc2d";
        } else {
            color = "#fe6055";
        }
        res.body +=
            "<a class=\"h-6 even:bg-gray-50 flex items-center text-sm cursor-pointer hover:bg-[#0156dd] hover:text-white\">\
                <div class=\"h-5 w-5 mx-2 rounded-full bg-[" +
            color +
            "] border-[1px] border-gray-200 mr-1\"></div>\
                <b style=\"color: " +
            color + "\">" + std::to_string((int)it->res.status_code) + ": " + it->res.reason + "</b>&nbsp;(" +
            it->req.method + " " + it->req.target + ", " + std::to_string(it->timestamp) +
            "s)\
            </a>";
    }
    res.body +=
        "</div>\
    </div>\
</body>\
\
</html>";
}

[[nodiscard]] double LoggerModule::GetHandlerPriority() const noexcept { return 0; }

[[nodiscard]] bool LoggerModule::ShouldHandle(const ziapi::http::Context &ctx, const ziapi::http::Request &req) const
{
    return req.method == "GET" && req.target == logs_route_;
}

DYLIB_API ziapi::IModule *LoadZiaModule() { return new LoggerModule(); }
