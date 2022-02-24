#include "http/HTTPRequestStreamParser.hpp"

#include <sstream>
#include <stdexcept>

bool HTTPRequestStreamParser::Done() { return state_ == DONE; }

const ziapi::http::Request &HTTPRequestStreamParser::GetRequest() { return output_; }

void HTTPRequestStreamParser::Clear()
{
    state_ = NONE;
    last_chunk_size_ = -1;
    last_header_key_.clear();
    buffer_.clear();
}

std::size_t HTTPRequestStreamParser::Feed(char *data, std::size_t size)
{
    std::size_t parsed_bytes = 0;
    std::size_t total_parsed_bytes = 0;
    std::size_t old_buffer_size = 0;

    buffer_.insert(buffer_.end(), data, data + size);
    old_buffer_size = buffer_.size();

    do {
        parsed_bytes = (this->*parsers_[state_])();  // total bytes parsed thanks to the function call, taking into
                                                     // account those already stored
        total_parsed_bytes += parsed_bytes;
    } while (parsed_bytes && state_ != DONE);

    return state_ == DONE ? old_buffer_size - total_parsed_bytes
                          : size;  // either returns whole size or remaining (stored) bytes after a request completion
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
    std::string tmp_buffer;
    Version &version = output_.version;
    std::size_t eol = buffer_.find(CRLF);
    std::size_t bytes_parsed = 0;
    float fversion = -1;

    if (eol == buffer_.npos)
        return 0;
    // Checks HTTP/ prefix aswell as if the number is apart of the Version enum

    bytes_parsed = NextWord(tmp_buffer, CRLF);
    if (tmp_buffer.starts_with(http_prefix) == false)
        throw std::invalid_argument("Invalid HTTP version prefix");
    std::istringstream(tmp_buffer.substr(http_prefix.length())) >> fversion;

    // A number is not considered apart of an enum if it's out of the enum's bounds
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
    std::size_t tmp_bytes = 0;

    if (headers.find("Content-Length") != headers.end() && is_number(headers["Content-Length"])) {
        // If Content-Length (body size) is specified
        std::size_t content_length = 0;

        std::istringstream(headers["Content-Length"]) >> content_length;
        if (buffer_.size() >= content_length) {
            buffer_ = buffer_.substr(0, content_length);
            bytes_parsed = content_length;
            state_ = DONE;
        }
    } else if (headers.find("Transfer-Encoding") != headers.end() && headers["Transfer-Encoding"] == "chunked") {
        // If the body is sent in chunks
        do {
            tmp_bytes = ParseBodyChunk();
            bytes_parsed += tmp_bytes;
        } while (tmp_bytes && state_ != DONE);
    } else {
        throw std::invalid_argument(
            "Invalid body transmission. Specify either Content-Length or set the Transfer-Enconding header to "
            "\"chunked\"");
    }
    return bytes_parsed;
}

std::size_t HTTPRequestStreamParser::ParseBodyChunk()
{
    auto is_number = [](const std::string &s) {
        return std::all_of(s.begin(), s.end(), [](const char &c) { return std::isdigit(c); });
    };
    auto eol = buffer_.find(CRLF);
    std::string tmp_buffer;
    std::size_t bytes_parsed = 0;
    auto &body = output_.body;

    if (eol == buffer_.npos)
        return bytes_parsed;
    bytes_parsed = NextWord(tmp_buffer, CRLF);
    if (last_chunk_size_ < 0) {
        if (!is_number(tmp_buffer))
            throw std::invalid_argument("Wrong size in chunked body");
        std::istringstream(tmp_buffer) >> last_chunk_size_;
    } else {
        if (tmp_buffer.size() != last_chunk_size_)
            throw std::invalid_argument("Body chunk is smaller than specified size");
        if (last_chunk_size_ == 0)
            state_ = DONE;
        body.insert(body.end(), tmp_buffer.begin(), tmp_buffer.end());
        last_chunk_size_ = -1;
    }
    return bytes_parsed;
}