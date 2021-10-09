#include <webcrown/webcrown.hpp>

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
    std::shared_ptr<webcrown::server::service> sv =
            std::make_shared<webcrown::server::service>();

    std::shared_ptr<http::http_server> http =
            std::make_shared<http::http_server>(sv,
                                                8005,
                                                "127.0.0.1");


    //  curl -v -X POST http://localhost:8080/upload \
    -F "upload[]=@/Users/alex/GolandProjects/multipart/avatar_avatar.jpg" \
    -H "Content-Type: multipart/form-data" \
    -H 'Expect:'

    auto router_middleware = std::make_shared<http::routing_middleware>();

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

    auto route2 = std::make_shared<route>(http_method::get, "/",
                                          [](http_request const& request, http_response& response)
    {
        auto raw = R"({"data": "ola"})";

        response.set_body(raw);
        response.set_status(http_status::ok);
    });

    router_middleware->add_router(route1);
    router_middleware->add_router(route2);

    http->add_middleware(router_middleware);

    asio::error_code ec;
    sv->start(ec);

    // verify error

    while(!sv->is_started())
        pthread_yield();

    http->start();

    while(!http->is_started())
        pthread_yield();

    for(;;)
    {
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }

    sv->stop(ec);
}
