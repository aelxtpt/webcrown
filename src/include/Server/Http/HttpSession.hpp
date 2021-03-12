#pragma once
#include "Server/Session.hpp"
#include "Server/Http/HttpRequest.hpp"
#include "Server/Http/IController.hpp"

namespace webcrown {
namespace server {
namespace http {

class HttpServer;

class HttpSession : public Session, public std::enable_shared_from_this<HttpSession>
{
  HttpRequest request_;

  std::vector<std::shared_ptr<IController>> controllers_;
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

  void controllers(std::vector<std::shared_ptr<IController>> const& controllers) { controllers_ = controllers; }
};

}}}
