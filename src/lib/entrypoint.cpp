#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include "webcrown_http.hpp"
#include "webcrown/server/http/middlewares/routing_middleware.hpp"

namespace http = webcrown::server::http;

int main()
{
    try
    {
        // curl --verbose --cacert server.crt --location --request GET 'https://localhost:8443'
	    const unsigned short port_num = 8443;

        auto context = std::make_shared<asio::ssl::context>(asio::ssl::context::tlsv12);
        context->set_password_callback(
      [](std::size_t max_length, asio::ssl::context::password_purpose purpose) -> std::string
        {
        //return "Mag@d4sCAROhPapai";
            return "qwerty";
        });
        context->use_certificate_chain_file("server.pem");
        context->use_private_key_file("server.pem", asio::ssl::context::pem);
        context->use_tmp_dh_file("dh4096.pem");

        webcrown::webcrown_http crown("127.0.0.1", port_num, context);

        auto router_middleware = std::make_shared<http::routing_middleware>();
        router_middleware->add_router(std::make_shared<http::route>("/user/1"),
                                      [](http::http_request const& request,
                                          http::http_response& response)
        {
        });

        crown.server()->add_middleware(router_middleware);
        crown.start();

        for(;;)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        crown.stop();

        std::cout << "fim\n";

  }
  catch(std::exception& ex)
  {
    std::cout << ex.what() << ".\n";
  }
}
