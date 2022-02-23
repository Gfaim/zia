#include "../../include/http/HTTPRequestStreamParser.hpp"  // what u lookin at

#include <sstream>
#include <stdexcept>

bool HTTPRequestStreamParser::Done() { return !_output.empty(); }

const ziapi::http::Request &HTTPRequestStreamParser::GetRequest() { return _output.back(); }

// TODO: Reset buffer aswell ?
void HTTPRequestStreamParser::Clear() { _state = NONE; }

std::size_t HTTPRequestStreamParser::Feed(char *data, [[gnu::unused]] std::size_t size)
{
    std::size_t bytes_used = 0;

    _buffer += data;
    do {
        bytes_used += (this->*_parsers[_state])();
    } while (bytes_used);

    return bytes_used;
}
std::size_t HTTPRequestStreamParser::handleNone(void)
{
    if (_state == NONE && !_buffer.empty()) {
        _state = METHOD;
        _output.emplace_back(ziapi::http::Request{});
    }
    return 0;
}

std::size_t HTTPRequestStreamParser::parseMethod(void)
{
    std::size_t bytes_parsed = NextWord(_output.back().method, " ");

    if (bytes_parsed)
        _state = TARGET;
    return bytes_parsed;
}

std::size_t HTTPRequestStreamParser::parseTarget(void)
{
    std::size_t bytes_parsed = NextWord(_output.back().target, " ");

    if (bytes_parsed)
        _state = VERSION;
    return bytes_parsed;
}

std::size_t HTTPRequestStreamParser::parseHeaderKey(void)
{
    std::string key;
    std::size_t bytes_parsed = NextWord(key, ":");

    if (bytes_parsed) {
        _lastHeaderKey = key;
    }
    _state = HEADER_VALUE;
    return bytes_parsed;
}

std::size_t HTTPRequestStreamParser::parseHeaderValue(void)
{
    std::string value;
    std::string end_of_headers_delim = CRLF + CRLF;
    std::size_t end_of_headers = value.find(end_of_headers_delim);
    std::size_t eol = value.find(CRLF);
    std::size_t bytes_parsed = 0;
    bool last_header = (eol == end_of_headers);

    if (eol == value.npos)
        return bytes_parsed;

    bytes_parsed = NextWord(value, last_header ? end_of_headers_delim : CRLF);

    _output.back().fields[_lastHeaderKey] = value;

    if (last_header) {
        _lastHeaderKey.clear();
        _state = BODY;
    } else
        _state = HEADER_KEY;

    return bytes_parsed;
}

std::size_t HTTPRequestStreamParser::parseVersion(void)
{
    using Version = ziapi::http::Version;

    static const std::string http_prefix = "HTTP/";
    std::string tmp_buffer;
    Version &version = _output.back().version;
    std::size_t eol = _buffer.find(CRLF);
    std::size_t bytes_parsed = 0;
    float fversion = -1;

    if (eol == _buffer.npos)
        return 0;
    bytes_parsed = NextWord(tmp_buffer, CRLF);
    if (tmp_buffer.starts_with(http_prefix) == false)
        throw std::invalid_argument("Invalid HTTP version prefix");
    std::istringstream(tmp_buffer.substr(http_prefix.length())) >> fversion;
    if (fversion < static_cast<std::size_t>(Version::kV1) || fversion > static_cast<std::size_t>(Version::kV3))
        throw std::invalid_argument("Invalid HTTP version number");
    version = Version(fversion * 10);
    _state = HEADER_KEY;
    return bytes_parsed;
}

std::size_t HTTPRequestStreamParser::NextWord(std::string &res, const std::string &delim)
{
    std::size_t end_of_word = _buffer.find(delim);
    std::size_t bytes_parsed = 0;
    std::size_t to_skip = delim.length();

    if (end_of_word != _buffer.npos) {
        res = _buffer.substr(0, end_of_word);
        _buffer = _buffer.substr(end_of_word + to_skip);
        bytes_parsed = res.size();
    }
    return bytes_parsed;
}
