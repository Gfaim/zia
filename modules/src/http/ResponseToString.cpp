#include "http/ResponseToString.hpp"

#include <iomanip>
#include <sstream>

std::string ResponseToString(const ziapi::http::Response &r)
{
    float version = static_cast<float>(r.version) / 10.f;
    std::string version_str;
    std::string res;
    std::stringstream version_stream;

    version_stream << std::fixed << std::setprecision(1) << version;
    version_str = version_stream.str();
    res = "HTTP/" + version_str + " " + std::to_string(static_cast<std::size_t>(r.status_code)) + " " +
          kStatusCodeMap.at(r.status_code) + "\r\n";
    for (const auto &[key, value] : r.headers) {
        res += key + ": " + value + "\r\n";
    }
    res += "\r\n";
    res += r.body;
    return res;
}
