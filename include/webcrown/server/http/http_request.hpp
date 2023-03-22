#pragma once
#include "webcrown/server/http/http_method.hpp"
#include <any>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

namespace webcrown {
namespace server {
namespace http {

using std::vector;
using std::unordered_map;

struct http_form_upload
{
    std::shared_ptr<std::vector<std::byte>> bytes;
    unordered_map<std::string, std::string> headers;
};

class http_context
{
    using context_name = std::string;
    using context_value = std::any;
    unordered_map<context_name, context_value> _values;
public:

    void add_value(std::pair<context_name, context_value> const& v)
    {
        if(_values.empty())
            _values.insert(v);
        else
            _values[v.first] = v.second;
    }

    decltype(_values) values() const noexcept { return _values; }
};

class http_request
{
    http_method method_;
    int protocol_version;
    std::string target_;
    std::unordered_map<std::string, std::string> headers_;
    std::string body_;
    std::vector<http_form_upload> uploads_;
public:
    explicit http_request(
        http_method method,
        int protocol_version,
        std::string_view target,
        std::unordered_map<std::string, std::string> const& headers,
        std::string_view body = std::string{})
        : method_(method)
        , protocol_version(protocol_version)
        , target_(target)
        , headers_(headers)
        , body_(body)
    {}
    
    explicit http_request(
                          http_method method,
                          int protocol_version,
                          std::string_view target,
                          std::unordered_map<std::string, std::string> const& headers,
                          std::vector<http_form_upload> form_upload)
        : method_(method)
        , protocol_version(protocol_version)
        , target_(target)
        , headers_(headers)
        , uploads_(std::move(form_upload))
    {}

    http_method method() const noexcept { return method_; }

    std::string target() const noexcept { return target_; }

    std::unordered_map<std::string, std::string> headers() const noexcept { return headers_; }

    std::string body() const noexcept { return body_; }
    
    std::vector<http_form_upload> uploads() const noexcept { return uploads_; }
};

}}}
