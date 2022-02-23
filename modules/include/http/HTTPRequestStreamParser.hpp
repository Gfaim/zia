#include <cstdint>
#include <vector>

#include "ziapi/Http.hpp"

/// Stateful parser for HTTP 1.1 requests messages.
/// Feed bytes to the parser till a request is formed.
class HTTPRequestStreamParser {
public:
    /// Feed bytes to the parser.
    /// @throws Throws upon parsing failure.
    /// @returns The number of bytes utilized. If that number is lower than size it means the request has been
    /// successfully parsed. In this case, all subsequent calls will return 0 and be idempotent.
    std::size_t Feed(char *data, std::size_t size);

    /// Done indicates whether the underlying request object is complete.
    bool Done();

    /// GetRequest returns a read-only reference to the underlying request object.
    const ziapi::http::Request &GetRequest();

    /// Reset the state of the parser.
    void Clear();

private:
    static const inline std::string CRLF = "\r\n";

    std::size_t handleNone();
    std::size_t parseMethod();
    std::size_t parseTarget();
    std::size_t parseVersion();
    std::size_t parseHeaderKey();
    std::size_t parseHeaderValue();
    std::size_t parseBody();
    std::size_t parseDone();

    // Function pointer type that returns true if the current state has finished being parsed
    using ParseHandler = std::size_t (HTTPRequestStreamParser::*)(void);

    enum RequestState : ushort { NONE, METHOD, TARGET, VERSION, HEADER_KEY, HEADER_VALUE, BODY, DONE, COUNT } _state;

    ParseHandler _parsers[RequestState::COUNT] = {
        &HTTPRequestStreamParser::handleNone,     &HTTPRequestStreamParser::parseMethod,
        &HTTPRequestStreamParser::parseTarget,    &HTTPRequestStreamParser::parseVersion,
        &HTTPRequestStreamParser::parseHeaderKey, &HTTPRequestStreamParser::parseHeaderValue,
        &HTTPRequestStreamParser::parseBody,      &HTTPRequestStreamParser::parseDone};

    std::size_t NextWord(std::string &res, const std::string &delim);

    std::string _lastHeaderKey;

    std::string _buffer;
    std::vector<ziapi::http::Request> _output;
};
