#pragma once
#include <deque>
#include <concepts>

namespace webcrown {
namespace server {
namespace http {

class http_request;
class http_response;

//template<typename T>
//concept middleware_base = requires(T c) {
//    { c.on_setup() } -> std::same_as<void>;
//    { c.xpto() } -> std::same_as<void>;
//};

class middleware
{
public:
    virtual ~middleware() = default;

    virtual void on_setup(http_request const& request, http_response& response) = 0;
};

struct cors_middleware
{

};

struct authentication{};


}}}
