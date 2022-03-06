#include "DirectoryListing.hpp"

#include <ziapi/Logger.hpp>

#include "DirectoryHtmlFactory.hpp"
#include "FileHtmlFactory.hpp"
#include "dylib/dylib.hpp"

void DirectoryListingModule::Init(const ziapi::config::Node &cfg)
{
    try {
        root_ = cfg["modules"]["directoryListing"]["root"].AsString();

        if (root_.length() > 1 && root_[root_.length() - 1] == '/')
            root_.pop_back();
    } catch (const std::exception &) {
        ziapi::Logger::Warning("Couldn't load configuration. Default root will be used: ", root_);
    }
}

[[nodiscard]] double DirectoryListingModule::GetHandlerPriority() const noexcept { return 0.5f; }

[[nodiscard]] bool DirectoryListingModule::ShouldHandle(const ziapi::http::Context &,
                                                        const ziapi::http::Request &req) const
{
    return req.method == ziapi::http::method::kGet;
}

void DirectoryListingModule::Handle(ziapi::http::Context &, const ziapi::http::Request &req, ziapi::http::Response &res)

{
    auto target = req.target;

    if (target[0] == '/')
        target.erase(0, 1);
    auto filepath = std::filesystem::path(root_) / std::filesystem::path(target);
    std::error_code ec;
    res.headers[ziapi::http::header::kContentType] = "text/html";

    if (std::filesystem::is_directory(filepath, ec)) {
        DirectoryHtmlFactory fac(filepath);
        res.body = fac.GetHtml();
    } else if (std::filesystem::is_regular_file(filepath)) {
        FileHtmlFactory fac(filepath);
        res.body = fac.GetHtml();
    } else {
        res.status_code = ziapi::http::Code::kNotFound;
        res.reason = ziapi::http::reason::kNotFound;
    }
}
DYLIB_API ziapi::IModule *LoadZiaModule() { return new DirectoryListingModule(); }
