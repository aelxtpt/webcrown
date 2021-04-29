#pragma once
#include <deque>
#include <concepts>
#include "webcrown/server/http/http_request.hpp"
#include "webcrown/server/http/http_response.hpp"

namespace webcrown {
namespace server {
namespace http {

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
