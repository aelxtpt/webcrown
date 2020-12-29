#include "Server/Http/HttpServer.hpp"


namespace webcrown {
namespace server {
namespace http {

HttpServer::HttpServer(
  std::shared_ptr<spdlog::logger> logger,
  std::shared_ptr<Service> const& service,
  uint16_t port_num,
  std::string_view address)
  : Server(logger, service, port_num, address)
{
  
}

}}}
