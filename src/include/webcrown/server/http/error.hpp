#ifndef WEBCROWN_ERROR_HPP
#define WEBCROWN_ERROR_HPP

#include <system_error>

namespace webcrown {
namespace server {
namespace http {

enum class http_error : std::uint8_t
{
    unknown = 0,

    /// When the request line is incomplete
    incomplete_request_line,

    ///
    bad_method,

    bad_target,

    bad_version
};

class http_error_category : public std::error_category
{
public:
    const char* name() const noexcept override
    {
        return "webcrown http error";
    }

    std::string message(int ec) const override
    {
        switch(static_cast<http_error>(ec))
        {
            case http_error::incomplete_request_line:
                return "request line is incomplete";
            default:
                return "unknown webcrown http error";
        }
    }

};

/// Create an std::error_code from the http error
/// \param ec http error code
/// \return std error code
inline
std::error_code
make_error(http_error ec);

}}}

#include "error.inl"

#endif //WEBCROWN_ERROR_HPP
