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
    const unsigned short port_num = 8001;

    webcrown::WebCrown crown("127.0.0.1", port_num);
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
