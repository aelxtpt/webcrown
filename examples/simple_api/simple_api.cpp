#include "webcrown/server/http/http_method.hpp"
#include "webcrown/server/http/http_request.hpp"
#include "webcrown/server/http/http_response.hpp"
#include "webcrown/server/http/http_server.hpp"
#include "webcrown/server/http/middlewares/auth_middleware.hpp"
#include "webcrown/server/http/middlewares/cors/cors_middleware.hpp"
#include "webcrown/server/http/middlewares/route.hpp"
#include "webcrown/server/http/middlewares/routing_middleware.hpp"
#include "webcrown/server/http/status.hpp"
#include "webcrown/server/service.hpp"

using std::make_shared;
namespace http = webcrown::server::http;

std::vector<spdlog::sink_ptr>
init_logger()
{
    std::vector<spdlog::sink_ptr> sinks;

    auto console_sink = make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::debug);
    console_sink->set_pattern("%d/%m/%Y %H:%M:%S.%e.%f %i [%^%l%$] [thread %t] %v");

    sinks.push_back(console_sink);

    return sinks;
}

void
home(http::http_request const& request, http::http_response& response, http::path_parameters_type const& parameters, http::http_context const& context)
{
    response.set_status(http::http_status::ok);
    response.set_body("Hello world");
}

int main()
{
    try
    {
        auto logger = webcrown::setup_logger(init_logger());

        spdlog::set_level(spdlog::level::level_enum::debug); // No effect for the library.

        auto log = spdlog::get(webcrown::logger_name);
        if(!log)
        {
            printf("Fail log");
            exit(1);
        }

        auto server = make_shared<webcrown::server::service>();
        auto http = make_shared<http::http_server>(server, 8080, "127.0.0.1");

        auto router_middleware = make_shared<http::routing_middleware>();
        auto cors_middleware = make_shared<http::cors_middleware>();

        router_middleware->add_router(make_shared<http::route>(http::http_method::get, "/", home));

        http->add_middleware(cors_middleware);
        http->add_middleware(router_middleware);

        asio::error_code ec;
        server->start(ec);

        while(!server->is_started())
            sched_yield();

        printf("Asio service was started!");

        http->start();

        while(!http->is_started())
            sched_yield();

        for(;;)
            std::this_thread::sleep_for(std::chrono::nanoseconds(50));

        server->stop(ec);
    }
    catch(std::exception const& ex)
    {
        printf("Error on server %s", ex.what());
    }
}