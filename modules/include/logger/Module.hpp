#include <iomanip>
#include <sstream>

#include "ziapi/Logger.hpp"
#include "ziapi/Module.hpp"

class LoggerModule : virtual public ziapi::IPreProcessorModule, public ziapi::IPostProcessorModule {
public:
    void Init(const ziapi::config::Node &config) {}

    [[nodiscard]] ziapi::Version GetVersion() const noexcept override { return {3, 1, 0}; }

    [[nodiscard]] ziapi::Version GetCompatibleApiVersion() const noexcept override { return {3, 1, 0}; }

    [[nodiscard]] const char *GetName() const noexcept override { return "LoggerModule"; }

    [[nodiscard]] const char *GetDescription() const noexcept override
    {
        return "Log all responses from HTTP requests";
    }

    [[nodiscard]] double GetPostProcessorPriority() const noexcept override;

    [[nodiscard]] bool ShouldPostProcess(const ziapi::http::Context &ctx,
                                         const ziapi::http::Response &res) const override;

    [[nodiscard]] double GetPreProcessorPriority() const noexcept override;

    [[nodiscard]] bool ShouldPreProcess(const ziapi::http::Context &ctx,
                                        const ziapi::http::Request &req) const override;

    void PostProcess(ziapi::http::Context &ctx, ziapi::http::Response &res) override;

    void PreProcess(ziapi::http::Context &ctx, ziapi::http::Request &req) override;
};