#include <iomanip>
#include <sstream>
#include <mutex>

#include "ziapi/Logger.hpp"
#include "ziapi/Module.hpp"

class LoggerModule : virtual public ziapi::IPreProcessorModule,
                     virtual public ziapi::IPostProcessorModule,
                     virtual public ziapi::IHandlerModule {
private:
    struct RequestInfos {
        double timestamp;
        ziapi::http::Request req;
        ziapi::http::Response res;
    };

    std::mutex mu_{};

    std::string logs_route_{};
    std::vector<RequestInfos> requests_{};
    std::uint32_t requests_buffer_size_{100};

public:
    void Init(const ziapi::config::Node &) override;

    [[nodiscard]] ziapi::Version GetVersion() const noexcept override { return {3, 1, 0}; }

    [[nodiscard]] ziapi::Version GetCompatibleApiVersion() const noexcept override { return {3, 1, 0}; }

    [[nodiscard]] const char *GetName() const noexcept override { return "LoggerModule"; }

    [[nodiscard]] const char *GetDescription() const noexcept override
    {
        return "Log all responses from HTTP requests";
    }

    [[nodiscard]] double GetHandlerPriority() const noexcept override;

    [[nodiscard]] bool ShouldHandle(const ziapi::http::Context &ctx, const ziapi::http::Request &req) const override;

    [[nodiscard]] double GetPostProcessorPriority() const noexcept override;

    [[nodiscard]] bool ShouldPostProcess(const ziapi::http::Context &ctx, const ziapi::http::Request &req,
                                         const ziapi::http::Response &res) const override;

    [[nodiscard]] double GetPreProcessorPriority() const noexcept override;

    [[nodiscard]] bool ShouldPreProcess(const ziapi::http::Context &ctx,
                                        const ziapi::http::Request &req) const override;

    void Handle(ziapi::http::Context &ctx, const ziapi::http::Request &req, ziapi::http::Response &res) override;

    void PostProcess(ziapi::http::Context &ctx, const ziapi::http::Request &req, ziapi::http::Response &res) override;

    void PreProcess(ziapi::http::Context &ctx, ziapi::http::Request &req) override;
};
