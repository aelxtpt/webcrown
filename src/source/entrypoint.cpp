#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include "server/asio.h"

using std::shared_ptr;

int main() {
  try
  {
    constexpr unsigned short port_num = 3333;
    asio::io_context ioctx;

    asio::ip::tcp protocol = asio::ip::tcp::v4();

    asio::ip::tcp::acceptor acceptor(ioctx);

    asio::error_code ec;

    acceptor.open(protocol, ec);

    if(ec.value() != 0) {
      std::cout << "Failed to open the acceptor socket!"
        << " Error code = "
        << ec.value() << ". Message: "
        << ec.message();
    }

    ioctx.run();

  }
  catch(std::exception& ex)
  {
    std::cout << ex.what() << ".\n";
  }
}
