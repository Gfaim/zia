#include "../../include/http/HTTPRequestStreamParser.hpp"  // what u lookin at

#include <sstream>
#include <stdexcept>

// TODO: really sure it's "Done" ? what if someone parses > 1 requests without getting them
bool HTTPRequestStreamParser::Done() { return !_output.empty(); }

const ziapi::http::Request &HTTPRequestStreamParser::GetRequest() { return _output.back(); }

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
        _output.emplace_back();
    }
    return 0;
}

std::size_t HTTPRequestStreamParser::parseMethod(void)
{
    std::string &method = _output.back().method;
    std::size_t first_space = _buffer.find(' ');

    if (first_space == _buffer.npos)
        return 0;
    method = _buffer.substr(0, first_space);
    _buffer = _buffer.substr(first_space + 1);
    _state = TARGET;
}

std::size_t HTTPRequestStreamParser::parseTarget(void)
{
    std::string &target = _output.back().target;
    std::size_t first_space = _buffer.find(' ');

    if (first_space == _buffer.npos)
        return 0;
    target = _buffer.substr(0, first_space);
    _buffer = _buffer.substr(first_space + 1);
    _state = VERSION;
}

std::size_t HTTPRequestStreamParser::parseVersion(void)
{
    using Version = ziapi::http::Version;

    Version &version = _output.back().version;
    std::size_t eol = _buffer.find(CRLF);
    float fversion = -1;

    if (eol == _buffer.npos)
        return 0;
    if (_buffer.substr(0, 5) != "HTTP/")
        throw std::invalid_argument("Invalid HTTP version prefix");
    _buffer = _buffer.substr(5);
    std::istringstream(_buffer) >> fversion;
    if (fversion <= 0)
        throw std::invalid_argument("Invalid HTTP version number");
    version = Version(fversion * 10);
    eol = _buffer.find(CRLF);
    _buffer = _buffer.substr(eol + 2);
    _state = HEADER_KEY;
}