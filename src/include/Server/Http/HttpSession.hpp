#pragma once
#include "Server/Session.hpp"

namespace webcrown {
namespace server {
namespace http {

class HttpServer;

class HttpSession : public Session
{
public:
  explicit HttpSession(
    uint64_t session_id,
    std::shared_ptr<HttpServer> server,
    std::shared_ptr<spdlog::logger> const& logger,
    std::shared_ptr<asio::ssl::context> const& context);

  HttpSession(HttpSession const&) = delete;
  HttpSession(HttpSession &&) = delete;

  HttpSession& operator=(HttpSession const&) = delete;
  HttpSession& operator=(HttpSession&&) = delete;

  ~HttpSession() = default;

  void on_received(void const* buffer, std::size_t size) override;
};

}}}
