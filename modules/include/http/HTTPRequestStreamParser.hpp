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

    std::size_t ParseMethod();
    std::size_t ParseTarget();
    std::size_t ParseVersion();
    std::size_t ParseHeaderKey();
    std::size_t ParseHeaderValue();
    std::size_t ParseBody();
    std::size_t ParseDone();

    // Function pointer type that returns true if the current state has finished being parsed
    using ParseHandler = std::size_t (HTTPRequestStreamParser::*)(void);

    enum RequestState : unsigned short { METHOD, TARGET, VERSION, HEADER_KEY, HEADER_VALUE, BODY, DONE, COUNT } state_;

    ParseHandler parsers_[RequestState::COUNT] = {
        &HTTPRequestStreamParser::ParseMethod,      &HTTPRequestStreamParser::ParseTarget,
        &HTTPRequestStreamParser::ParseVersion,     &HTTPRequestStreamParser::ParseHeaderKey,
        &HTTPRequestStreamParser::ParseHeaderValue, &HTTPRequestStreamParser::ParseBody,
        &HTTPRequestStreamParser::ParseDone};

    std::size_t ParseBodyChunk();
    int last_chunk_size_ = -1;

    /// Gets every bytes until `delim`. It will remove every one of them from the buffer,
    /// including the leading `delim` bytes.
    /// @returns The number of bytes utilized.
    /// @param res A reference to where the result will be stored
    /// @param delim The string that delimits the end of the word
    std::size_t NextWord(std::string &res, const std::string &delim);

    std::string last_header_key_;

    std::string buffer_;
    ziapi::http::Request output_{};
};
