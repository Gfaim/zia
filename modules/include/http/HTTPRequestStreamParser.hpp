#include <cstdint>

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
};
