#pragma once

#include "status.hpp"
#include <string>

namespace webcrown {
namespace server {
namespace http {

///
/// Response = Status-Line
///             *(general-header | response-header | entity header) CLRF
///             CLRF
///             [ message body ]
///
///
class http_response
{
    std::string buffer_;
    http_status status_;
    std::string body_;

    /// TODO: At the moment, we not check if the sent headers are valids.
    /// The developer can be send anything
    /// Parse it on rules :)
    std::unordered_map<std::string, std::string> headers_;
public:
    http_response();

    void set_status(http_status status) noexcept { status_ = status; }

    void add_header(std::string_view key, std::string_view value);

    void set_body(std::string_view body);

    std::string body() const noexcept { return body_; }
    http_status status() const noexcept { return status_; }

    static http_response ok();
    static http_response bad_request();
    
    /// Status-Line
    ///     Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
    std::string build();
};

/// Constructor
inline
http_response::http_response()
    : status_(http_status::internal_server_error) // The developer don't setted the status on response
{}

inline
std::string
http_response::build()
{
    // This will be the first step, so clean buffer
    buffer_.clear();

    // TODO: add checking if the status, etc are filled

    auto status_reason = make_status(status_);

    // HTTP-Version
    buffer_.append("HTTP/1.1");
    // SP
    buffer_.append(" ");
    // Status Code
    buffer_.append(std::to_string(status_reason.first));
    // SP
    buffer_.append(" ");
    // Reason-Phrase
    buffer_.append(status_reason.second);
    // CRLF
    buffer_.append("\r\n");

    // Content-Length
    if (!body_.empty())
    {
        add_header("Content-Length", std::to_string(body_.size()));
    }

    // Headers
    for(auto const& header : headers_)
    {
        buffer_.append(header.first);
        buffer_.append(": ");
        buffer_.append(header.second);
        buffer_.append("\r\n");
    }

    // CRLF
    buffer_.append("\r\n");

    // CRLF
    buffer_.append("\r\n");

    // Body
    buffer_.append(body_);


    return buffer_;
}

inline
void
http_response::add_header(std::string_view key, std::string_view value)
{
    headers_.emplace(key, value);
}

inline
void
http_response::set_body(std::string_view body)
{
    body_ = body;
}

}}}
