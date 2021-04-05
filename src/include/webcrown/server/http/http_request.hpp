#pragma once
#include "webcrown/server/http/method.hpp"
#include <unordered_map>

namespace webcrown {
namespace server {
namespace http {

class http_request
{
    method method_;
    int protocol_;
    std::string target_;
    std::unordered_map<std::string, std::string> headers_;
    std::string body_;
public:
    explicit http_request(
        method method,
        int protocol,
        std::string_view target,
        std::unordered_map<std::string, std::string> const& headers,
        std::string_view body)
        : method_(method)
        , protocol_(protocol)
        , headers_(headers)
        , body_(body)
    {}

    method method() const noexcept { return method_; }
};

}}}