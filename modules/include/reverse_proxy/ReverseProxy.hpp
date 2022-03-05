#include <string>
#include <ziapi/Module.hpp>

class ReverseProxy : public ziapi::IHandlerModule {
public:
    ReverseProxy() : address_(), port_(), enable_tls_(false) {}

    void Init(const ziapi::config::Node &cfg) override
    {
        const auto &reverse_proxy_config = cfg["modules"]["reverseProxy"].AsDict();
        const auto &tls_it = reverse_proxy_config.find("tls");

        if (tls_it != reverse_proxy_config.end()) {
            enable_tls_ = tls_it->second->AsBool();
        }
        address_ = reverse_proxy_config.at("address")->AsString();
        port_ = reverse_proxy_config.at("port")->AsInt();
    }

    [[nodiscard]] ziapi::Version GetVersion() const noexcept override { return {1, 0, 0}; }

    [[nodiscard]] ziapi::Version GetCompatibleApiVersion() const noexcept override { return {5, 0, 0}; }

    [[nodiscard]] const char *GetName() const noexcept override { return "reverse_proxy"; }

    [[nodiscard]] const char *GetDescription() const noexcept override { return "basic proxy to mirror tls/ssl sites"; }

    [[nodiscard]] double GetHandlerPriority() const noexcept override { return 0.5f; }

    [[nodiscard]] bool ShouldHandle(const ziapi::http::Context &, const ziapi::http::Request &) const override
    {
        return true;
    }

    void Handle(ziapi::http::Context &ctx, const ziapi::http::Request &req, ziapi::http::Response &res) override;

private:
    std::string address_;

    std::uint16_t port_;

    bool enable_tls_;
};
