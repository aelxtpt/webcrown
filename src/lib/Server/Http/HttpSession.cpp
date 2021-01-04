#include "Server/Http/HttpSession.hpp"
#include "Server/Http/HttpServer.hpp"

namespace webcrown {
namespace server {
namespace http {

HttpSession::HttpSession(uint64_t session_id,
  std::shared_ptr<HttpServer> server,
  std::shared_ptr<spdlog::logger> const& logger,
  std::shared_ptr<asio::ssl::context> const& context)
  : Session(session_id, server, logger, context)
{
}

void HttpSession::on_received(void const* buffer, std::size_t size)
{
  int brk = 0;
}

}}}