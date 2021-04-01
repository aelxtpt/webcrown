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
    std::unordered_map<std::string, std::string> headers_;
    std::string body_;
public:
    explicit http_request(
        std::basic_string_view<char> method,
        int protocol,
        std::unordered_map<std::string, std::string> const& headers,
        std::string_view body)
    {}

    method method1() const noexcept { return method_; }
};

}}}