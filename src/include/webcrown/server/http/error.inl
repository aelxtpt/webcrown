#ifndef WEBCROWN_ERROR_INL
#define WEBCROWN_ERROR_INL

namespace webcrown {
namespace server {
namespace http {

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

#endif //WEBCROWN_ERROR_INL
