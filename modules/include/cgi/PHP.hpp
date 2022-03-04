#pragma once

#include "ACGI.hpp"

class PHPCGI : public ACGI {
public:
    PHPCGI() : ACGI() { _bin_path = "/usr/bin/php-cgi"; };
    ~PHPCGI() = default;

    bool ShouldHandle(const http::Context &, const http::Request &req) const
    {
        std::string tmp_target = req.target;

        if (tmp_target.find('?') != tmp_target.npos)
            tmp_target = tmp_target.substr(0, tmp_target.find('?'));

        return tmp_target.ends_with(".php");
    }

    [[nodiscard]] Version GetVersion() const noexcept { return {0, 1, 1}; }

    [[nodiscard]] Version GetCompatibleApiVersion() const noexcept { return {5, 0, 0}; }

    [[nodiscard]] const char *GetName() const noexcept { return "php-cgi"; }

    [[nodiscard]] const char *GetDescription() const noexcept { return "Executes PHP (CGI Protocol)."; }

    [[nodiscard]] double GetHandlerPriority() const noexcept { return 0.3f; }

protected:
private:
};
