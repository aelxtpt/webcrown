#pragma once
#include "Server/Session.hpp"

class HttpServer;

namespace webcrown {
namespace server {
namespace http {

class HttpSession : public Session
{
public:
  explicit HttpSession(
    uint64_t session_id,
    std::shared_ptr<HttpServer> const& server,
    std::shared_ptr<spdlog::logger> const& logger)
    : Session(session_id, std::dynamic_pointer_cast<Server>(server), logger)
  {}

  HttpSession(HttpSession const&) = delete;
  HttpSession(HttpSession &&) = delete;

  HttpSession& operator=(HttpSession const&) = delete;
  HttpSession& operator=(HttpSession&&) = delete;

  ~HttpSession() = default;

  void on_received(void const* buffer, std::size_t size) override;
};

}}}
