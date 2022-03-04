#pragma once

#include "../cgi/ACGI.hpp"

class RubyCGI : public ACGI {
public:
    RubyCGI() : ACGI()
    {
        _bin_path = "/usr/bin/ruby";
        headers_env.push_back("Negotiate");
        headers_env.push_back("Pragma");
    };

    ~RubyCGI() = default;

    bool ShouldHandle(const http::Context &, const http::Request &req) const
    {
        std::string tmp_target = req.target;

        if (tmp_target.find('?') != tmp_target.npos)
            tmp_target = tmp_target.substr(0, tmp_target.find('?'));

        return tmp_target.ends_with(".rb");
    }

    [[nodiscard]] Version GetVersion() const noexcept { return {0, 1, 1}; }

    [[nodiscard]] Version GetCompatibleApiVersion() const noexcept { return {5, 0, 0}; }

    [[nodiscard]] const char *GetName() const noexcept { return "ruby-cgi"; }

    [[nodiscard]] const char *GetDescription() const noexcept { return "Executes Ruby (CGI Protocol)."; }

    [[nodiscard]] double GetHandlerPriority() const noexcept { return 0.3f; }

protected:
private:
};
