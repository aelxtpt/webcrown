#pragma once
#include "webcrown/server/server.hpp"
#include "webcrown/server/service.hpp"
#include "webcrown/server/http/http_middleware.hpp"

namespace webcrown {
namespace server {
namespace http {

class http_session;

class http_server : public server
{
  /// This is an object representing SSL context. Basically
  /// this is a wrapper around the SSL_CTX data structure defined
  /// by OpenSSL library
  std::shared_ptr<asio::ssl::context> context_;

  middlewares_
public:
  explicit http_server(
      std::shared_ptr<spdlog::logger> logger,
      std::shared_ptr<webcrown::server::service> const& service,
      uint16_t port_num,
      std::string_view address,
      std::shared_ptr<asio::ssl::context> const& context);

  http_server(http_server const&) = delete;
  http_server(http_server&&) = delete;

  http_server& operator=(http_server const&) = delete;
  http_server& operator=(http_server&&) = delete;

  ~http_server() = default;

  // SslServer interface
private:
  std::shared_ptr<session> create_session(
    uint64_t session_id,
    std::shared_ptr<server> const& server,
    std::shared_ptr<spdlog::logger> const& logger) override;
};

}}}
