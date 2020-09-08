#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include "server/asio.h"

using std::shared_ptr;

int main() {
  try {
    const unsigned short PortNum = 2222;

    std::cout << "Webcrown 0.0.1\n";
    std::cout << "Running on port: " << PortNum << "\n";

    webcrown::server::SimpleHttpServer Server;

    unsigned int ThreadPoolSize = std::thread::hardware_concurrency();

    Server.start(8888, ThreadPoolSize);

    std::this_thread::sleep_for(std::chrono::seconds(60));

    Server.stop();

    std::cout << "Shutdown aplication" << std::endl;

  }
  catch(std::exception& ex)
  {
    std::cout << ex.what() << ".\n";
  }
}
