#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include "server/asio.h"

using std::shared_ptr;

int main() {
  try {
    const unsigned short PortNum = 2222;
    unsigned int ThreadPoolSize = std::thread::hardware_concurrency();

    std::cout << "Webcrown 0.0.1\n";
    std::cout << "Running on port: " << PortNum << "\n";
    std::cout << "Thread Pool Size: " << ThreadPoolSize << "\n";

    webcrown::server::SimpleHttpServer Server;
    Server.start(PortNum, ThreadPoolSize);

    std::this_thread::sleep_for(std::chrono::seconds(60));

    Server.stop();

    std::cout << "Shutdown aplication" << std::endl;

  }
  catch(std::exception& ex)
  {
    std::cout << ex.what() << ".\n";
  }
}
