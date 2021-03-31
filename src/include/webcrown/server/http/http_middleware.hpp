#pragma once
#include <deque>

namespace webcrown {
namespace server {
namespace http {

class http_request;
class http_response;

template<typename T>
concept middleware
{
    { c.on_setup() } -> void;
    { c.xpto() } -> void;
};

template<typename middlewareT>
class middlewares
{
    std::deque<middlewareT> middlewares_;
public:

    void add(middlewareT const& middleware)
    { middlewares_.push_front(middleware); }

    void invoke(http_request const& request, http_response& response)
    {
        for(auto it = middlewares_.crbegin(); it < middlewares_.crend(); ++it)
        {
            it->on_setup(request, response);
        }
    }
};

struct routing_middleware
{

};

struct cors_middleware
{

};

struct authentication{};


}}}
