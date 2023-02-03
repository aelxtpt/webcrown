#include "asio/error_code.hpp"
#include "webcrown/server/http/http_method.hpp"
#include "webcrown/server/http/http_request.hpp"
#include "webcrown/server/http/http_response.hpp"
#include "webcrown/server/http/http_server.hpp"
#include "webcrown/server/http/middlewares/route.hpp"
#include "webcrown/server/http/middlewares/routing_middleware.hpp"
#include "webcrown/server/service.hpp"
#include <chrono>
#include <ratio>
#include <sched.h>
#include <thread>
#include <webcrown/webcrown.hpp>
#include <memory>

namespace http = webcrown::server::http;
using webcrown::server::http::http_method;
using webcrown::server::http::http_request;
using webcrown::server::http::http_response;

using std::shared_ptr;
using std::make_shared;

int main()
{
    auto sv = make_shared<webcrown::server::service>();
    auto http = make_shared<http::http_server>(sv, 8005, "127.0.0.1");

    // Simple route
    auto router_middleware = make_shared<http::routing_middleware>();

    auto route1 = make_shared<http::route>(http_method::get, "/test",
    [](http_request const& request, http_response& response)
    {
        
    });

    router_middleware->add_router(route1);

    http->add_middleware(router_middleware);

    asio::error_code ec;
    sv->start(ec);

    while(!sv->is_started())
        sched_yield();

    http->start();

    while(!http->is_started())
        sched_yield();

    for(;;)
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));

    sv->stop(ec);
}