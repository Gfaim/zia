#pragma once

#include "../cgi/ACGI.hpp"

class PerlCGI : public ACGI {
public:
    PerlCGI() : ACGI(true) { _bin_path = "/usr/bin/perl"; };

    ~PerlCGI() = default;

    bool ShouldHandle(const http::Context &, const http::Request &req) const
    {
        std::string tmp_target = req.target;

        if (tmp_target.find('?') != tmp_target.npos)
            tmp_target = tmp_target.substr(0, tmp_target.find('?'));

        return tmp_target.starts_with("/perl/") && tmp_target.ends_with(".pl");
    }

    [[nodiscard]] Version GetVersion() const noexcept { return {0, 1, 1}; }

    [[nodiscard]] Version GetCompatibleApiVersion() const noexcept { return {5, 0, 0}; }

    [[nodiscard]] const char *GetName() const noexcept { return "perl-cgi"; }

    [[nodiscard]] const char *GetDescription() const noexcept { return "Executes Perl (CGI Protocol)."; }

    [[nodiscard]] double GetHandlerPriority() const noexcept { return 0.3f; }

protected:
private:
};
