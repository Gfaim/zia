#pragma once

#include <any>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>
#include <unordered_map>

#include "ziapi/Config.hpp"
#include "ziapi/Logger.hpp"
#include "ziapi/Module.hpp"

using namespace ziapi;

#if defined(WIN32) || defined(_WIN64) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define CGI_WIN
#include <strsafe.h>
#include <windows.h>
#define POPEN _popen
#define PCLOSE _pclose
#define FGETS fgets
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
        const auto get_cfg_string = [&cfg, this](const std::string &key, std::string &variable) {
            try {
                auto &dict = cfg["modules"][GetName()].AsDict();

                variable = dict.at(key)->AsString();
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

        if (res.body.empty() && headers.empty())
            InternalError(res, "CGI Error.");
        else {
            ProcessHeaders(headers, res.headers);
            ProcessStatus(res);

            ziapi::Logger::Info(res.body);
        }
    }

protected:
    using unique_ptr_pipe_t = std::unique_ptr<FILE, decltype(&PCLOSE)>;

    ACGI(bool cmd_specify_file = false) : _cmd_specify_file(cmd_specify_file){};

    void InternalError(http::Response &res, const std::string &body = "")
    {
        res.status_code = ziapi::http::Code::kInternalServerError;
        res.reason = ziapi::http::reason::kInternalServerError;
        res.body = "<center><h1>500 - Internal server error</h1><p>" + body + "</p></center>";
    }

    // Execute the cgi command and returns the headers and body as a string pair
    std::pair<std::string, std::string> ExecuteCGICommand(const http::Request &req, const http::Context &ctx)
    {
        auto escape = [](const std::string &str) { return std::regex_replace(str, std::regex("\""), "\\\""); };
        auto envMap = GenerateEnv(req, ctx);
        std::string command;
#ifdef CGI_WIN
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
        env.emplace_back('\0');

        command += "echo \"" + escape(req.body) + "\" | " + _bin_path;
        if (_cmd_specify_file) {
            command += " " + envMap["SCRIPT_FILENAME"];
        }

        if (!CreateProcess(NULL,
                           command.data(),  // command line
                           NULL,            // process security attributes
                           NULL,            // primary thread security attributes
                           TRUE,            // handles are inherited
                           0,               // creation flags
                           env.data(),      // use parent's environment
                           NULL,            // use parent's current directory
                           &start_info,     // STARTUPINFO pointer
                           &proc_info)) {   // receives PROCESS_INFORMATION
            return {};
        } else {
            CloseHandle(proc_info.hProcess);
            CloseHandle(proc_info.hThread);
            CloseHandle(child_output_w);
        }
        return ProcessOutput(child_output_r);
    }
#else
        command += "echo \"" + escape(req.body) + "\"" + " | env -i ";

        for (const auto &[key, value] : envMap) {
            command += key + "=\"" + escape(value) + "\" ";
        }
        command += _bin_path;

        if (_cmd_specify_file)
            command += " " + envMap["SCRIPT_FILENAME"];

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
        DWORD bytes_read;
#else
    std::pair<std::string, std::string> ProcessOutput(unique_ptr_pipe_t &pipe) const
    {
#endif
        std::array<char, kBufferSize> buffer;
        std::string result;
        std::string headers;
        std::string body;

#ifdef CGI_WIN
        while (ReadFile(pipe_read, buffer.data(), static_cast<DWORD>(buffer.size()), &bytes_read, NULL) && bytes_read) {
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
            while (FGETS(buffer.data(), static_cast<int>(buffer.size()), command_pipe.get()) != nullptr) {
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
        const std::string tmp_target =
            req.target.find('?') != std::string::npos ? req.target.substr(0, req.target.find('?')) : req.target;
        const auto filepath = std::filesystem::path(_root) / std::filesystem::path(tmp_target.substr(1));
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
        env["REMOTE_ADDR"] = get_ctx_value("client.socket.address");
        env["REMOTE_PORT"] = get_ctx_value("client.socket.port");
        env["SERVER_NAME"] = get_ctx_value("http.server.name");
        env["SERVER_PORT"] = get_ctx_value("http.server.port");
        env["SERVER_PROTOCOL"] = get_ctx_value("http.server.protocol");
        env["SERVER_SOFTWARE"] = _version;
        env["DOCUMENT_ROOT"] = _root;

        return env;
    }

    std::vector<std::string> headers_env{"Content-Length", "Content-Type",    "Accept",       "Accept-Encoding",
                                         "Accept-Charset", "Accept-Language", "Host",         "From",
                                         "Referer",        "User-Agent",      "Cache-Control"};
    std::string _bin_path{};
    std::string _root = "/var/www/";
    std::string _version{};

    bool _cmd_specify_file;
};
