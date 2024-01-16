#pragma once
#include <string_view>
#include <cstdint>


namespace webcrown {
namespace server {
namespace http {

enum class http_method : std::uint8_t
{
    unknown = 0,

    post,
    get,
    delete_,
    options,
    patch
    // TODO: add other methods
};

/// Converts HTTP http_method enum to the HTTP get_method string
/// \param m get_method enum
/// \return HTTP get_method as string
inline
std::string_view
to_string(http_method m);

/// Converts a HTTP http_method string to the HTTP get_method enum
/// \param m string get_method
/// \return HTTP get_method as enum
inline
http_method
to_method(std::string_view m);

}}}

#include "method.inl"
