#pragma once

#include <system_error>

namespace webcrown {
namespace server {
namespace http {

enum class http_error : std::uint8_t
{
    unknown = 0,

    /// When the request line is incomplete
    incomplete_start_line,

    invalid_request_line,

    incomplete_message_header,

    invalid_message_header_crlf,

    ///
    bad_method,

    bad_target,

    bad_version,

    bad_field,

    bad_field_value,

    content_type_not_implemented,

    no_boundary_header_for_multipart,

    need_more
};

class http_error_category : public std::error_category
{
public:
    const char* name() const noexcept override
    {
        return "webcrown_http http error";
    }

    std::string message(int ec) const override
    {
        switch(static_cast<http_error>(ec))
        {
            case http_error::incomplete_start_line:
                return "http incomplete start line";
            case http_error::invalid_request_line:
                return "http invalid request line";
            case http_error::incomplete_message_header:
                return "http incomplete message header";
            case http_error::invalid_message_header_crlf:
                return "http invalid message header crlf";
            case http_error::bad_method:
                return "http bad get_method";
            case http_error::bad_target:
                return "http bad target";
            case http_error::bad_version:
                return "http bad version";
            case http_error::bad_field:
                return "http bad field";
            case http_error::bad_field_value:
                return "http bad field value";
            case http_error::content_type_not_implemented:
                return "http content-type unsupported";
            case http_error::no_boundary_header_for_multipart:
                return "no_boundary_header_for_multipart";
            default:
                return "unknown webcrown_http http error";
        }
    }

};

/// Create an std::error_code from the http error
/// \param ec http error code
/// \return std error code
inline
std::error_code
make_error(http_error ec)
{
    // If T is a complete enumeration (enum) type, provides a member typedef
    // type that names the underlying type of T.
    // Otherwise, the behavior is undefined.

    static http_error_category const cat{};
    return std::error_code{static_cast<std::underlying_type_t<http_error>>(ec), cat};
}

}}}