#include <webcrown/webcrown.hpp>
#include <webcrown/server/http/middlewares/routing_middleware.hpp>

#include <chrono>
#include <thread>
#include <stdlib.h>
#include <fstream>

namespace http = webcrown::server::http;
using webcrown::server::http::route;
using webcrown::server::http::http_method;
using webcrown::server::http::http_request;
using webcrown::server::http::http_response;
using webcrown::server::http::http_status;

int main()
{
    webcrown::webcrown_http http("127.0.0.1", 8115);

    //  curl -v -X POST http://localhost:8080/upload \
    -F "upload[]=@/Users/alex/GolandProjects/multipart/avatar_avatar.jpg" \
    -H "Content-Type: multipart/form-data" \
    -H 'Expect:'

    auto router_middleware = std::make_shared<http::routing_middleware>(http.logger());

    auto route1 = std::make_shared<route>(http_method::post, "/upload",
    [](http_request const &request, http_response &response)
    {
        auto&& uploads = request.uploads();
        
        printf("My uploads: %lu\n", uploads.size());
        
        uint8_t img_count = 1;
        for(auto const& upload : uploads)
        {
            auto file_name = "/home/alexlima/Downloads/images/image_" + std::to_string(img_count) + ".jpg";
            auto fs = std::ofstream(file_name, std::ios_base::out | std::ios_base::app | std::ios_base::binary);
            
            printf("Writing file %s\n", file_name.c_str());
            
            fs.write((const char*)upload.bytes->data(), upload.bytes->size());
            
            img_count++;
        }

        response.set_status(http_status::ok);
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
