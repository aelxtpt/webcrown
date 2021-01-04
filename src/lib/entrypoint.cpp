#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include "WebCrown.hpp"


#include <asio.hpp>

using std::shared_ptr;

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
      [](size_t max_length, asio::ssl::context::password_purpose purpose) -> std::string
      {
        //return "Mag@d4sCAROhPapai";
        return "123456";
      });
    context->use_certificate_chain_file("cert.pem");
    context->use_private_key_file("key.pem", asio::ssl::context::pem);
    context->use_tmp_dh_file("/home/aex/Projects/certificates/dh4096.pem");

    webcrown::WebCrown crown("127.0.0.1", port_num, context);
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