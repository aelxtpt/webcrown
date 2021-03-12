#include "Server/Http/HttpServer.hpp"
#include "Server/Http/HttpSession.hpp"

namespace webcrown {
namespace server {
namespace http {

HttpServer::HttpServer(
    std::shared_ptr<spdlog::logger> logger,
    std::shared_ptr<Service> const& service,
    uint16_t port_num,
    std::string_view address,
    std::shared_ptr<asio::ssl::context> const& context,
    const std::vector<std::shared_ptr<IController> >& controllers)
  : Server(logger, service, port_num, address, context)
  , context_(context)
  , controllers_(controllers)
{
  
}

std::shared_ptr<Session>
HttpServer::create_session(uint64_t session_id,
  std::shared_ptr<Server> const& server,
  const std::shared_ptr<spdlog::logger>& logger)
{
  auto session = std::make_shared<HttpSession>(
        session_id,
        std::dynamic_pointer_cast<HttpServer>(server),
        logger,
        context_);

  session->controllers(controllers_);

  return session;
}

}}}
