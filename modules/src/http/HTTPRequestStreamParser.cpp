#include "http/HTTPRequestStreamParser.hpp"

#include <sstream>
#include <stdexcept>

bool HTTPRequestStreamParser::Done() { return state_ == DONE; }

const ziapi::http::Request &HTTPRequestStreamParser::GetRequest() { return output_; }

// TODO: Reset buffer aswell ?
void HTTPRequestStreamParser::Clear() { state_ = NONE; }

std::size_t HTTPRequestStreamParser::Feed(char *data, std::size_t size)
{
    std::size_t parsed_bytes = 0;
    std::size_t total_parsed_bytes = 0;
    std::size_t oldbuffer_size_ = 0;

    buffer_.insert(buffer_.end(), data, data + size);
    oldbuffer_size_ = buffer_.size();

    do {
        parsed_bytes = (this->*parsers_[state_])();
        total_parsed_bytes += parsed_bytes;
    } while (parsed_bytes && state_ != DONE);

    return state_ == DONE ? oldbuffer_size_ - total_parsed_bytes : size;
}

std::size_t HTTPRequestStreamParser::handleNone(void)
{
    if (!buffer_.empty()) {
        state_ = METHOD;
        output_ = ziapi::http::Request{};
    }
    return 0;
}

std::size_t HTTPRequestStreamParser::parseMethod(void)
{
    std::size_t bytes_parsed = NextWord(output_.method, " ");

    if (bytes_parsed)
        state_ = TARGET;
    return bytes_parsed;
}

std::size_t HTTPRequestStreamParser::parseTarget(void)
{
    std::size_t bytes_parsed = NextWord(output_.target, " ");

    if (bytes_parsed)
        state_ = VERSION;
    return bytes_parsed;
}

std::size_t HTTPRequestStreamParser::parseHeaderKey(void)
{
    std::string key;
    std::size_t bytes_parsed = NextWord(key, ":");

    if (bytes_parsed) {
        last_header_key_ = key;
        state_ = HEADER_VALUE;
    }  // else if check if string has wrong characters, throw
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

    output_.fields[last_header_key_] = value;

    if (last_header) {
        last_header_key_.clear();
        state_ = BODY;
    } else
        state_ = HEADER_KEY;

    return bytes_parsed;
}

std::size_t HTTPRequestStreamParser::parseVersion(void)
{
    using Version = ziapi::http::Version;

    static const std::string http_prefix = "HTTP/";
    std::string tmpbuffer_;
    Version &version = output_.version;
    std::size_t eol = buffer_.find(CRLF);
    std::size_t bytes_parsed = 0;
    float fversion = -1;

    if (eol == buffer_.npos)
        return 0;
    bytes_parsed = NextWord(tmpbuffer_, CRLF);
    if (tmpbuffer_.starts_with(http_prefix) == false)
        throw std::invalid_argument("Invalid HTTP version prefix");
    std::istringstream(tmpbuffer_.substr(http_prefix.length())) >> fversion;
    if (fversion < static_cast<std::size_t>(Version::kV1) || fversion > static_cast<std::size_t>(Version::kV3))
        throw std::invalid_argument("Invalid HTTP version number");
    version = Version(fversion * 10);
    state_ = HEADER_KEY;
    return bytes_parsed;
}

std::size_t HTTPRequestStreamParser::NextWord(std::string &res, const std::string &delim)
{
    std::size_t end_of_word = buffer_.find(delim);
    std::size_t bytes_parsed = 0;
    std::size_t to_skip = delim.length();

    if (end_of_word != buffer_.npos) {
        res = buffer_.substr(0, end_of_word);
        buffer_ = buffer_.substr(end_of_word + to_skip);
        bytes_parsed = res.size();
    }
    return bytes_parsed;
}

std::size_t HTTPRequestStreamParser::parseBody()
{
    auto is_number = [](const std::string &s) {
        return std::all_of(s.begin(), s.end(), [](const char &c) { return std::isdigit(c); });
    };
    auto &headers = output_.fields;
    std::size_t bytes_parsed = 0;

    if (headers.find("Content-Length") != headers.end() && is_number(headers["Content-Length"])) {
        std::size_t content_length = 0;
        std::istringstream(headers["Content-Length"]) >> content_length;

        if (buffer_.size() >= content_length) {
            buffer_ = buffer_.substr(0, content_length);
            // buffer_size_ -= content_length;
            bytes_parsed = content_length;
        }
        return bytes_parsed;
    } else if (headers.find("Transfer-Encoding") != headers.end() && headers["Transfer-Encoding"] == "chunked") {
        // TODO : read <size><CRLF> and <body_chunk><CRLF> until having 0<CRLF><CRLF> (watch out, maybe a <body_chunk>
        // has a 0<CRLF><CRLF>)
        return bytes_parsed;
    }

    throw std::invalid_argument(
        "Invalid body transmission. Specify either Content-Length or set the Transfer-Enconding header to \"chunked\"");
}