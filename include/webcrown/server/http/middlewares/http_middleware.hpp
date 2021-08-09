#pragma once
#include <deque>
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

    // TODO: Gambeta temporaria, para retornar um http response antes de processar outro middleware
    virtual bool should_return_now() = 0;
    virtual void should_return_now(bool flag) = 0;
};

}}}
