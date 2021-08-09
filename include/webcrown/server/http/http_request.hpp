#pragma once
#include "webcrown/server/http/http_method.hpp"
#include <unordered_map>
#include <string>

namespace webcrown {
namespace server {
namespace http {

class http_request
{
    http_method method_;
    int protocol_version;
    std::string target_;
    std::unordered_map<std::string, std::string> headers_;
    std::string body_;
public:
    explicit http_request(
        http_method method,
        int protocol_version,
        std::string_view target,
        std::unordered_map<std::string, std::string> const& headers,
        std::string_view body)
        : method_(method)
        , protocol_version(protocol_version)
        , target_(target)
        , headers_(headers)
        , body_(body)
    {}

    http_method method() const noexcept { return method_; }

    std::string target() const noexcept { return target_; }

    std::unordered_map<std::string, std::string> headers() const noexcept { return headers_; }

    std::string body() const noexcept { return body_; }
};

}}}
