#pragma once
#include "Server/Server.hpp"
#include "Server/Http/HttpSession.hpp"

namespace webcrown {
namespace server {
namespace http {

class HttpServer : public Server
{
public:
  explicit HttpServer(
    std::shared_ptr<spdlog::logger> logger,
    std::shared_ptr<Service> const& service,
    uint16_t port_num,
    std::string_view address);

  HttpServer(HttpServer const&) = delete;
  HttpServer(HttpServer&&) = delete;

  HttpServer& operator=(HttpServer const&) = delete;
  HttpServer& operator=(HttpServer&&) = delete;

  ~HttpServer() = default;

  // SslServer interface
private:
  std::shared_ptr<Session> create_session(
    uint64_t session_id, std::shared_ptr<Server> const& server, 
    std::shared_ptr<spdlog::logger> const& logger) override
    {
      return std::make_shared<HttpSession>(session_id, std::dynamic_pointer_cast<HttpServer>(server), logger);
    }
};

}}}
