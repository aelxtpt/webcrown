#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include "WebCrown.hpp"
#include "Server/Http/HttpContext.hpp"

using std::shared_ptr;

class RegisterController : public webcrown::server::http::IController
{
  std::shared_ptr<webcrown::server::http::HttpContext> context_;
public:
  void on_setup(std::shared_ptr<webcrown::server::http::HttpContext> const& context) override
  {
    context_ = context;

    auto healthEndpopint = std::make_shared<webcrown::server::http::HttpEndpoint>("GET", "/health",
                                                         [](webcrown::server::http::HttpRequest const& request)
     {
        std::cout << "request was called\n";
     });

    context_->add_endpoint(healthEndpopint);
  }

  std::vector<std::shared_ptr<webcrown::server::http::HttpEndpoint>> endpoints() const override
  {
    return context_->endpoints();
  }
};

//class MyController : public webcrown::server::Controller
//{
//public:
//  void on_setup() override
//  {
//    req_->add_endpoint(http::GET, "/health",
//      [](HttpResponse const& response)
//      {
//
//      });
//  }
//};

int main() {
  try {
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

    std::vector<std::shared_ptr<webcrown::server::http::IController>> controllers;
    auto register_controller = std::make_shared<RegisterController>();
    controllers.push_back(register_controller);

    webcrown::WebCrown crown("127.0.0.1", port_num, context, controllers);
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
