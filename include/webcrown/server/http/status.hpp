#pragma once

#include <string>
#include <type_traits>
#include <utility>

namespace webcrown {
namespace server {
namespace http {

enum class http_status : std::uint16_t
{
    continue_ = 100,
    switching_protocols = 101,
    ok = 200,
    created = 201,
    accepted = 202,
    non_authoritative_information = 203,
    no_content = 204,
    reset_content = 205,
    partial_content = 206,
    multiple_choices = 300,
    moved_permanently = 301,
    found = 302,
    see_other = 303,
    not_modified = 304,
    use_proxy = 305,
    temporary_redirect = 307,
    bad_request = 400,
    unauthorized = 401,
    payment_required = 402,
    forbidden = 403,
    not_found = 404,
    method_not_allowed = 405,
    not_acceptable = 406,
    proxy_authentication_required = 407,
    request_time_out = 408,
    conflict = 409,
    gone = 410,
    length_required = 411,
    precondition_failed = 412,
    request_entity_too_large = 413,
    request_uri_too_large = 414,
    unsupported_media_type = 415,
    requested_range_not_satisfiable = 416,
    expectation_failed = 417,
    internal_server_error = 500,
    not_implemented = 501,
    bad_gateway = 502,
    service_unavailable = 503,
    gateway_time_out = 504,
    http_version_not_supported = 505
    };

/// Make a http status
/// \param status - HTTP status code
/// \return a pair with the status and reason-phrase
inline
std::pair<std::uint16_t, std::string>
make_status(http_status status)
{
    auto&& v = static_cast<std::uint16_t>(status);

    switch(status)
    {
        case http_status::continue_:
	        return std::make_pair(v, "Continue");
        case http_status::switching_protocols:
	        return std::make_pair(v, "Switching Protocols");
        case http_status::ok:
            return std::make_pair(v, "OK");
        case http_status::created:
	        return std::make_pair(v, "Created");
        case http_status::accepted:
	        return std::make_pair(v, "Accepted");
        case http_status::non_authoritative_information:
	        return std::make_pair(v, "Non-Authoritative Information");
        case http_status::no_content:
	        return std::make_pair(v, "No Content");
        case http_status::reset_content:
	        return std::make_pair(v, "Reset Content");
        case http_status::partial_content:
	        return std::make_pair(v, "Partial Content");
        case http_status::multiple_choices:
	        return std::make_pair(v, "Multiple Choices");
        case http_status::moved_permanently:
	        return std::make_pair(v, "Moved Permanently");
        case http_status::found:
	        return std::make_pair(v, "Found");
        case http_status::see_other:
	        return std::make_pair(v, "See Other");
        case http_status::not_modified:
	        return std::make_pair(v, "Not Modified");
        case http_status::use_proxy:
	        return std::make_pair(v, "Use Proxy");
        case http_status::temporary_redirect:
	        return std::make_pair(v, "Temporary Redirect");
        case http_status::bad_request:
	        return std::make_pair(v, "Bad Request");
        case http_status::unauthorized:
	        return std::make_pair(v, "Unauthorized");
        case http_status::payment_required:
	        return std::make_pair(v, "Payment Required");
        case http_status::forbidden:
	        return std::make_pair(v, "Forbidden");
        case http_status::not_found:
	        return std::make_pair(v, "Not Found");
        case http_status::method_not_allowed:
	        return std::make_pair(v, "Method Not Allowed");
        case http_status::not_acceptable:
	        return std::make_pair(v, "Not Acceptable");
        case http_status::proxy_authentication_required:
	        return std::make_pair(v, "Proxy Authentication Required");
        case http_status::request_time_out:
	        return std::make_pair(v, "Request Timeout");
        case http_status::conflict:
	        return std::make_pair(v, "Conflict");
        case http_status::gone:
	        return std::make_pair(v, "Gone");
        case http_status::length_required:
	        return std::make_pair(v, "Length Required");
        case http_status::precondition_failed:
	        return std::make_pair(v, "Precondition Failed");
        case http_status::request_entity_too_large:
	        return std::make_pair(v, "Payload Too Large");
        case http_status::request_uri_too_large:
	        return std::make_pair(v, "URI Too Long");
        case http_status::unsupported_media_type:
	        return std::make_pair(v, "Unsupported Media Type");
        case http_status::requested_range_not_satisfiable:
	        return std::make_pair(v, "Range Not Satisfiable");
        case http_status::expectation_failed:
	        return std::make_pair(v, "Expectation Failed");
        case http_status::internal_server_error:
	        return std::make_pair(v, "Internal Server Error");
        case http_status::not_implemented:
	        return std::make_pair(v, "Not Implemented");
        case http_status::bad_gateway:
	        return std::make_pair(v, "Bad Gateway");
        case http_status::service_unavailable:
	        return std::make_pair(v, "Service Unavailable");
        case http_status::gateway_time_out:
	        return std::make_pair(v, "Gateway Timeout");
        case http_status::http_version_not_supported:
	        return std::make_pair(v, "HTTP Version Not Supported");
        default:
	        return std::make_pair(v, "Unknown");
    }
}

}}}
