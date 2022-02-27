#include "http/RequestToString.hpp"

#include <iomanip>
#include <sstream>

std::string RequestToString(const ziapi::http::Request &r)
{
    float version = static_cast<float>(r.version) / 10.f;
    std::string version_str;
    std::string res;
    std::stringstream version_stream;

    version_stream << std::fixed << std::setprecision(1) << version;
    version_str = version_stream.str();
    res = r.method + " " + r.target + " HTTP/" + version_str + "\r\n";
    for (const auto &[key, value] : r.headers) {
        res += key + ": " + value + "\r\n";
    }
    res += "\r\n";
    res += r.body;
    return res;
}
