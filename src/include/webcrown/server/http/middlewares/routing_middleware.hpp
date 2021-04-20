#pragma once
#include "webcrown/server/http/middlewares/http_middleware.hpp"
#include "webcrown/server/http/middlewares/route.hpp"
#include <functional>

namespace webcrown {
namespace server {
namespace http {

class routing_middleware : public middleware
{
    using route_callback = std::function<void(http_request const& request, http_response& response)>;

    std::unordered_map<std::shared_ptr<route>, route_callback> routers_;
public:
    routing_middleware() = default;

    routing_middleware(routing_middleware const&) = delete;
    routing_middleware(routing_middleware &&) = delete;

    routing_middleware& operator=(routing_middleware const&) = delete;
    routing_middleware& operator=(routing_middleware &&) = delete;

    void on_setup(http_request const& request, http_response& response) override
    {
        // find route

        // if not found
        // return http response 404

        // if find, invoke callback
    }

    void add_router(std::shared_ptr<route> const route, route_callback callback)
    {

    }
};

}}}