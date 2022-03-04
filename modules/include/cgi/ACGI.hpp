#pragma once

#include <any>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <regex>
#include <unordered_map>

#include "ziapi/Config.hpp"
#include "ziapi/Logger.hpp"
#include "ziapi/Module.hpp"

using namespace ziapi;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define CGI_WIN
#include <strsafe.h>
#include <windows.h>
#define POPEN _popen
#define PCLOSE _pclose
#define FGETS _fgets
#else
#define POPEN popen
#define PCLOSE pclose
#define FGETS fgets
#endif

class ACGI : public IHandlerModule {
protected:
    static const inline std::string kHeaderBodySeparator = "\r\n\r\n";
    static constexpr uint16_t kBufferSize = 1024;

public:
    void Init(const config::Node &cfg)
    {
        auto &dict = cfg["modules"]["config"][GetName()].AsDict();
        const auto get_cfg_string = [&cfg, &dict, this](const std::string &key, std::string &variable) {
            try {
                if (dict.find(key) != dict.end()) {
                    variable = dict.at(key)->AsString();
                }
            } catch (const std::exception &) {
                Logger::Warning(std::string(GetName()) + ": Couldn't load " + key + ". A default value will be used (" +
                                variable + ").");
            }
        };

        get_cfg_string("root", _root);
        get_cfg_string("bin_path", _bin_path);
        GetCgiVersion();
    }

    void Handle(http::Context &ctx, const http::Request &req, http::Response &res)
    {
        std::string headers;
        std::tie(res.body, headers) = ExecuteCGICommand(req, ctx);

        // keep
        if (res.body.empty() && headers.empty())
            InternalError(res);
        else {
            ProcessHeaders(headers, res.headers);
            ProcessStatus(res);
        }
    }

protected:
    using unique_ptr_pipe_t = std::unique_ptr<FILE, decltype(&PCLOSE)>;

    ACGI() = default;

    void InternalError(http::Response &res)
    {
        res.status_code = ziapi::http::Code::kInternalServerError;
        res.body = "500 - Internal server error";
    }

    // Execute the cgi command and returns the headers and body as a string pair
#ifdef CGI_WIN
    std::pair<std::string, std::string> ExecuteCGICommand(const http::Request &req, const http::Context &ctx)
    {
        auto envMap = GenerateEnv(req, ctx);
        std::vector<char> env;
        HANDLE child_output_w = NULL;
        HANDLE child_output_r = NULL;
        HANDLE parent_input_handle = NULL;
        SECURITY_ATTRIBUTES sec_attr;

        sec_attr.nLength = sizeof(SECURITY_ATTRIBUTES);
        sec_attr.bInheritHandle = TRUE;
        sec_attr.lpSecurityDescriptor = NULL;

        parent_input_handle = GetStdHandle(STD_INPUT_HANDLE);

        if (!parent_input_handle)
            return {};
        if (!CreatePipe(&child_output_r, &child_output_w, &sec_attr, 0))
            return {};
        if (!SetHandleInformation(child_output_r, HANDLE_FLAG_INHERIT, 0))
            return {};

        PROCESS_INFORMATION proc_info;
        STARTUPINFO start_info;
        BOOL success = FALSE;

        ZeroMemory(&proc_info, sizeof(PROCESS_INFORMATION));
        ZeroMemory(&start_info, sizeof(STARTUPINFO));
        start_info.cb = sizeof(STARTUPINFO);
        start_info.hStdError = child_output_w;
        start_info.hStdOutput = child_output_w;
        start_info.hStdInput = parent_input_handle;
        start_info.dwFlags |= STARTF_USESTDHANDLES;

        for (auto &[key, value] : envMap) {
            std::string env_var_with_trailing_zero = key + "=" + value + '\0';
            env.insert(env.end(), env_var_with_trailing_zero.begin(), env_var_with_trailing_zero.end());
        }
        env += '\0';

        if (!CreateProcess(NULL,
                       _bin_path.data(),  // command line
                       NULL,              // process security attributes
                       NULL,              // primary thread security attributes
                       TRUE,              // handles are inherited
                       0,                 // creation flags
                       env.data(),        // use parent's environment
                       NULL,              // use parent's current directory
                       &start_info,       // STARTUPINFO pointer
                       &proc_info) {      // receives PROCESS_INFORMATION
            return {};
        } else {
            CloseHandle(proc_info.hProcess);
            CloseHandle(proc_info.hThread);
            CloseHandle(child_output_w);
        }
        return ProcessOutput(child_output_r);
    }

#else

    std::pair<std::string, std::string> ExecuteCGICommand(const http::Request &req, const http::Context &ctx)
    {
        std::unordered_map<std::string, std::string> env = GenerateEnv(req, ctx);
        std::string command = "env -i ";

        for (const auto &[key, value] : env) {
            command += key + "=\"" + std::regex_replace(value, std::regex("\""), "\\\"") + "\" ";
        }
        command += _bin_path;

        Logger::Info(command);

        unique_ptr_pipe_t command_pipe(POPEN(command.c_str(), "r"), PCLOSE);

        if (!command_pipe)
            return {};
        return ProcessOutput(command_pipe);
    }
#endif

    // Split whole output into headers and body
#ifdef CGI_WIN
    std::pair<std::string, std::string> ProcessOutput(HANDLE pipe_read) const
    {
        unsigned int bytes_read = 0;
#else
    std::pair<std::string, std::string> ProcessOutput(unique_ptr_pipe_t &pipe) const
    {
#endif
        std::array<char, kBufferSize> buffer;
        std::string result;
        std::string headers;
        std::string body;

#ifdef CGI_WIN
        while (ReadFile(pipe_read, buffer.data(), buffer.size(), &bytes_read, NULL) && bytes_read) {
#else
        while (FGETS(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
#endif
            result += buffer.data();
        }
        if (!result.empty()) {
            const auto separatorIndex = result.find(kHeaderBodySeparator);

            if (separatorIndex != result.npos) {
                body = result.substr(separatorIndex + kHeaderBodySeparator.length());
                headers = result.substr(0, separatorIndex);
            } else {
                body = result;
            }
        }
        return std::make_pair(body, headers);
    }

    // Fills response's headers from CGI's response
    void ProcessHeaders(std::string &headers, std::map<std::string, std::string> &header_map) const
    {
        std::istringstream headers_stream(headers);
        std::string full_header;

        while (std::getline(headers_stream, full_header)) {
            if (full_header.empty())
                continue;
            const auto delimiter = full_header.find(':');
            const auto header_name = full_header.substr(0, delimiter);
            auto header_value = full_header.substr(delimiter + 2);

            if (header_value.back() == '\r')
                header_value.pop_back();

            header_map[header_name] = header_value;
        }
    }

    // Process status from CGI's result
    void ProcessStatus(ziapi::http::Response &res) const
    {
        std::size_t status_code = 0;

        if (res.headers.find("Status") == res.headers.end())
            return;

        std::stringstream status_ss(res.headers["Status"]);
        status_ss >> status_code;
        res.status_code = ziapi::http::Code(status_code);
    }

    // Retrieve CGI's version by `-v`
    void GetCgiVersion()
    {
        std::array<char, kBufferSize> buffer;
        unique_ptr_pipe_t command_pipe(POPEN((_bin_path + " -v").c_str(), "r"), PCLOSE);

        _version.clear();
        if (command_pipe) {
            while (FGETS(buffer.data(), buffer.size(), command_pipe.get()) != nullptr) {
                _version += buffer.data();
            }
        }
    }

    // Get Auth Type from header value (Basic, Bearer, etc...)
    const std::string GetAuthType(const std::string &authorization) const
    {
        const auto firstSpace = authorization.find(" ");

        return firstSpace != authorization.npos ? authorization.substr(0, firstSpace) : "";
    }

    // Processing HTTP headers that CGI uses
    void ParseEnvHeaders(std::unordered_map<std::string, std::string> &env,
                         const std::map<std::string, std::string> &headers) const
    {
        if (headers.find("Authorization") != headers.end())
            env["AUTH_TYPE="] = GetAuthType(headers.at("Authorization"));

        for (const std::string &header : headers_env) {
            std::string cgi_formatted = header;

            // Format headers as env variables
            std::transform(cgi_formatted.begin(), cgi_formatted.end(), cgi_formatted.begin(), ::toupper);
            if (header.find('-') != header.npos)
                cgi_formatted.replace(header.find('-'), 1, "_");
            if (header.substr(0, 7) != "Content")
                cgi_formatted = "HTTP_" + cgi_formatted;

            if (headers.find(header) != headers.end())
                env[cgi_formatted] = headers.at(header);
        }
    }

    // Generate environment variables for CGI application
    const std::unordered_map<std::string, std::string> GenerateEnv(const http::Request &req,
                                                                   const http::Context &ctx) const
    {
        std::size_t query_index = 0;
        std::unordered_map<std::string, std::string> env;
        const auto &headers = req.headers;
        const auto filepath = std::filesystem::path(_root) / std::filesystem::path(req.target.substr(1));
        std::string filepath_str = filepath.string();
        std::function<const std::string(const std::string &)> get_ctx_value = [&ctx](const std::string &key) {
            return ctx.find(key) != ctx.end() ? std::any_cast<std::string>(ctx.at(key)) : "";
        };

        // Basic variables to be defined
        env["REDIRECT_STATUS"] = "200";
        env["GATEWAY_INTERFACE"] = "CGI/1.1";
        env["PATH_INFO"] = "";
        env["PATH_TRANSLATED"] = "";

        // Request context
        ParseEnvHeaders(env, headers);
        query_index = req.target.find('?');
        if (query_index != std::string::npos)
            env["QUERY_STRING"] = req.target.substr(query_index + 1);
        env["SCRIPT_FILENAME"] = filepath_str;
        env["SCRIPT_NAME"] = filepath_str;
        env["REQUEST_METHOD"] = req.method;

        // Client/Server Context
        env["REMOTE_ADDR"] = get_ctx_value("client.addr");
        env["REMOTE_PORT"] = get_ctx_value("client.port");
        env["SERVER_NAME"] = get_ctx_value("server.name");
        env["SERVER_PORT"] = get_ctx_value("server.port");
        env["SERVER_PROTOCOL"] = get_ctx_value("server.protocol");
        env["SERVER_SOFTWARE"] = _version;
        env["DOCUMENT_ROOT"] = _root;

        return env;
    }

    std::vector<std::string> headers_env{"Content-Length", "Content-Type",    "Accept", "Accept-Encoding",
                                         "Accept-Charset", "Accept-Language", "Host",   "Referer",
                                         "User-Agent"};
    std::string _bin_path{};
    std::string _root = "/var/www/";
    std::string _version{};
};
