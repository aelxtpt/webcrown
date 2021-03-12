#pragma once
#include "Server/Server.hpp"
#include "Server/Http/IController.hpp"


namespace webcrown {
namespace server {
namespace http {

class HttpSession;

class HttpServer : public Server
{
  /// This is an object representing SSL context. Basically
  /// this is a wrapper around the SSL_CTX data structure defined
  /// by OpenSSL library
  std::shared_ptr<asio::ssl::context> context_;

  // TODO: for each session, we pass by copy all controllers ?? Oh my god
  std::vector<std::shared_ptr<IController>> controllers_;
public:
  explicit HttpServer(
      std::shared_ptr<spdlog::logger> logger,
      std::shared_ptr<Service> const& service,
      uint16_t port_num,
      std::string_view address,
      std::shared_ptr<asio::ssl::context> const& context,
      std::vector<std::shared_ptr<IController>> const& controllers);

  HttpServer(HttpServer const&) = delete;
  HttpServer(HttpServer&&) = delete;

  HttpServer& operator=(HttpServer const&) = delete;
  HttpServer& operator=(HttpServer&&) = delete;

  ~HttpServer() = default;

  // SslServer interface
private:
  std::shared_ptr<Session> create_session(
    uint64_t session_id,
    std::shared_ptr<Server> const& server,
    std::shared_ptr<spdlog::logger> const& logger) override;
};

}}}
