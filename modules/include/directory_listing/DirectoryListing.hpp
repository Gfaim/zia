#include <filesystem>
#include <fstream>
#include <sstream>

#include "ziapi/Config.hpp"
#include "ziapi/Module.hpp"

class DirectoryListingModule : public ziapi::IHandlerModule {
public:
    DirectoryListingModule() : root_("./") {}

    void Init(const ziapi::config::Node &cfg) override;

    [[nodiscard]] ziapi::Version GetVersion() const noexcept override { return {3, 1, 0}; }

    [[nodiscard]] ziapi::Version GetCompatibleApiVersion() const noexcept override { return {3, 1, 0}; }

    [[nodiscard]] const char *GetName() const noexcept override { return "DirectoryListing"; }

    [[nodiscard]] const char *GetDescription() const noexcept override
    {
        return "Give access to a filesystem over HTTP";
    }

    [[nodiscard]] double GetHandlerPriority() const noexcept override;

    [[nodiscard]] bool ShouldHandle(const ziapi::http::Context &ctx, const ziapi::http::Request &req) const override;

    void Handle(ziapi::http::Context &ctx, const ziapi::http::Request &req, ziapi::http::Response &res) override;

private:
    std::string root_;
};