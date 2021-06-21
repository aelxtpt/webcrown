#include "webcrown/server/http/http_server.hpp"
//#include "webcrown_http/server/http/http.hpp"
#include "webcrown/server/http/http_session.hpp"

namespace webcrown {
namespace server {
namespace http {

http_server::http_server(
    std::shared_ptr<spdlog::logger> logger,
    std::shared_ptr<webcrown::server::service> const& service,
    uint16_t port_num,
    std::string_view address)
  : server(logger, service, port_num, address)
//  , context_(context)
{
}

std::shared_ptr<session>
http_server::create_session(uint64_t session_id,
                            std::shared_ptr<server> const& server,
                            const std::shared_ptr<spdlog::logger>& logger)
{
  auto session = std::make_shared<http_session>(
        session_id,
        std::dynamic_pointer_cast<http_server>(server),
        logger);

  session->middlewares(middlewares_);

  return session;
}

void
http_server::add_middleware(std::shared_ptr<middleware> const middleware)
{
    middlewares_.push_front(middleware);
}

}}}
