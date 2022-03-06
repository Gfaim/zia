#include "ResponseStreamParser.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>

bool ResponseStreamParser::Done() { return state_ == kDone; }

const ziapi::http::Response &ResponseStreamParser::GetResponse() { return output_; }

void ResponseStreamParser::Clear()
{
    state_ = kVersion;
    last_chunk_size_ = -1;
    last_header_key_.clear();
    output_ = ziapi::http::Response{};
}

std::size_t ResponseStreamParser::Feed(char *data, std::size_t size)
{
    std::size_t parsed_bytes = 0;
    std::size_t total_parsed_bytes = 0;
    std::size_t old_buffer_size = buffer_.size();

    buffer_.insert(buffer_.end(), data, data + size);

    do {
        parsed_bytes = (this->*parsers_[state_])();  // total bytes parsed thanks to the function call, taking into
                                                     // account those already stored
        total_parsed_bytes += parsed_bytes;
    } while (parsed_bytes && state_ != kDone);

    return state_ == kDone ? total_parsed_bytes - old_buffer_size
                           : size;  // either returns whole size or bytes used for a request completion
}

std::size_t ResponseStreamParser::ParseStatusCode(void)
{
    std::string buffer;
    std::size_t bytes_parsed = NextWord(buffer, " ");

    if (bytes_parsed == 1)
        throw std::invalid_argument("invalid http status code");
    if (bytes_parsed) {
        try {
            output_.status_code = ziapi::http::Code(std::stoi(buffer));
            state_ = kReason;
        } catch (...) {
            output_.status_code = ziapi::http::Code::kInternalServerError;
        }
    }
    return bytes_parsed;
}

std::size_t ResponseStreamParser::ParseReason(void)
{
    std::size_t bytes_parsed = NextWord(output_.reason, CRLF);

    if (bytes_parsed == 2)
        throw std::invalid_argument("Specify a valid http target");
    if (bytes_parsed)
        state_ = kHeaderKey;
    return bytes_parsed;
}

std::size_t ResponseStreamParser::ParseHeaderKey(void)
{
    std::string key;
    std::size_t crlf_tmp = buffer_.find(CRLF);
    std::size_t tdots_tmp = buffer_.find(":");
    std::size_t bytes_parsed = 0;

    if (tdots_tmp < crlf_tmp)
        bytes_parsed = NextWord(key, ":");

    if (bytes_parsed) {
        if (bytes_parsed == 1)
            throw std::invalid_argument("Specify a valid header value");

        // Format header with lowercase and Capital first letters
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        std::size_t find_pos = 0;
        while (find_pos != std::string::npos) {
            if (find_pos && find_pos != key.size())
                find_pos++;
            if (std::isalpha(key[find_pos]) && std::islower(key[find_pos]))
                key[find_pos] = std::toupper(key[find_pos]);
            find_pos = key.find('-', find_pos);
        }
        last_header_key_ = key;
        state_ = kHeaderValue;
    } else if (crlf_tmp == 0) {
        bytes_parsed = NextWord(key, CRLF);
        state_ = kBody;
    }
    return bytes_parsed;
}

std::size_t ResponseStreamParser::ParseHeaderValue(void)
{
    std::string value;
    std::string end_of_headers_delim = CRLF + CRLF;
    std::size_t end_of_headers = buffer_.find(end_of_headers_delim);
    std::size_t eol = buffer_.find(CRLF);
    std::size_t bytes_parsed = 0;
    bool last_header = (eol == end_of_headers);

    if (eol == value.npos)
        return bytes_parsed;

    bytes_parsed = NextWord(value, last_header ? end_of_headers_delim : CRLF);

    if (value.front() == ' ')
        value = value.substr(1);

    output_.headers[last_header_key_] = value;

    if (last_header) {
        last_header_key_.clear();
        state_ = kBody;
    } else
        state_ = kHeaderKey;

    return bytes_parsed;
}

std::size_t ResponseStreamParser::ParseVersion(void)
{
    using Version = ziapi::http::Version;

    static const std::string http_prefix = "HTTP/";
    std::string tmp_buffer;
    Version &version = output_.version;
    std::size_t eol = buffer_.find(" ");
    std::size_t bytes_parsed = 0;
    float fversion = -1;

    if (eol == buffer_.npos)
        return 0;
    // Checks HTTP/ prefix aswell as if the number is apart of the Version enum

    bytes_parsed = NextWord(tmp_buffer, " ");
    if (tmp_buffer.starts_with(http_prefix) == false)
        throw std::invalid_argument("Invalid HTTP version prefix");
    std::istringstream(tmp_buffer.substr(http_prefix.length())) >> fversion;
    fversion *= 10;

    // A number is not considered apart of an enum if it's out of the enum's bounds
    if (fversion < static_cast<std::size_t>(Version::kV1) || fversion > static_cast<std::size_t>(Version::kV3))
        throw std::invalid_argument("Invalid HTTP version number");
    version = Version(fversion);
    state_ = kStatusCode;
    return bytes_parsed;
}

std::size_t ResponseStreamParser::NextWord(std::string &res, const std::string &delim)
{
    std::size_t end_of_word = buffer_.find(delim);
    std::size_t bytes_parsed = 0;
    std::size_t to_skip = delim.length();

    if (end_of_word != buffer_.npos) {
        res = buffer_.substr(0, end_of_word);
        buffer_ = buffer_.substr(end_of_word + to_skip);
        bytes_parsed = res.size() + to_skip;
    }
    return bytes_parsed;
}

std::size_t ResponseStreamParser::ParseBody()
{
    auto is_number = [](const std::string &s) {
        return std::all_of(s.begin(), s.end(), [](const char &c) { return std::isdigit(c); });
    };
    auto &headers = output_.headers;
    std::size_t bytes_parsed = 0;
    std::size_t tmp_bytes = 0;

    if (headers.find("Content-Length") != headers.end()) {
        if (!is_number(headers["Content-Length"]))
            throw std::invalid_argument("Invalid Content-Length");

        // If Content-Length (body size) is specified
        std::size_t content_length = 0;

        std::istringstream(headers["Content-Length"]) >> content_length;
        if (buffer_.size() >= content_length) {
            output_.body = buffer_.substr(0, content_length);
            buffer_ = buffer_.substr(content_length);
            bytes_parsed = content_length;
            state_ = kDone;
        }
    } else if (headers.find("Transfer-Encoding") != headers.end()) {
        if (headers["Transfer-Encoding"] != "chunked")
            throw std::invalid_argument("Unknown Transfer-Encoding");

        // If the body is sent in chunks
        do {
            tmp_bytes = ParseBodyChunk();
            bytes_parsed += tmp_bytes;
        } while (tmp_bytes && state_ != kDone);
    } else
        state_ = kDone;

    return bytes_parsed;
}

std::size_t ResponseStreamParser::ParseBodyChunk()
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
    if (!bytes_parsed && last_chunk_size_ != 0)
        return bytes_parsed;
    if (last_chunk_size_ < 0) {
        if (!is_number(tmp_buffer))
            throw std::invalid_argument("Wrong size in chunked body");
        std::istringstream(tmp_buffer) >> last_chunk_size_;
    } else {
        if (tmp_buffer.size() != static_cast<std::size_t>(last_chunk_size_))
            throw std::invalid_argument("Body chunk is smaller than specified size");

        if (last_chunk_size_ == 0)
            state_ = kDone;
        body.insert(body.end(), tmp_buffer.begin(), tmp_buffer.end());
        last_chunk_size_ = -1;
    }
    return bytes_parsed;
}
