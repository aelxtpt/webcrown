#include <webcrown/webcrown.hpp>
#include <webcrown/server/http/middlewares/routing_middleware.hpp>

#include <chrono>
#include <thread>

namespace http = webcrown::server::http;
using webcrown::server::http::route;
using webcrown::server::http::http_method;
using webcrown::server::http::http_request;
using webcrown::server::http::http_response;

int main()
{
    webcrown::webcrown_http http("127.0.0.1", 8090);

    //  curl -v -X POST http://localhost:8090/upload \
    -F "upload[]=@/Users/alex/GolandProjects/multipart/avatar_avatar.jpg" \
    -H "Content-Type: multipart/form-data" \
    -H 'Expect:'

    auto router_middleware = std::make_shared<http::routing_middleware>(http.logger());

    auto route1 = std::make_shared<route>(http_method::post, "/upload",
    [](http_request const &request, http_response &response)
    {
        int brk{};
    });


    router_middleware->add_router(route1);
    http.server()->add_middleware(router_middleware);

    http.start();

    for(;;)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    http.stop();
}
