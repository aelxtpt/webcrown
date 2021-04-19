#pragma once
#include "webcrown/server/session.hpp"
#include "webcrown/server/http/middlewares/http_middleware.hpp"


namespace webcrown {
namespace server {
namespace http {

class http_server;

class http_session :
	public webcrown::server::session,
	public std::enable_shared_from_this<http_session>
{
    std::deque<std::shared_ptr<middleware>> middlewares_;
public:
  explicit http_session(
      uint64_t session_id,
      std::shared_ptr<http_server> server,
      std::shared_ptr<spdlog::logger> const& logger,
      std::shared_ptr<asio::ssl::context> const& context);

  http_session(http_session const&) = delete;
  http_session(http_session &&) = delete;

  http_session& operator=(http_session const&) = delete;
  http_session& operator=(http_session&&) = delete;

  ~http_session() = default;

  void middlewares(std::deque<std::shared_ptr<middleware>> const& middlewares) noexcept
  { middlewares_ = middlewares; }

  void on_received(void const* buffer, std::size_t size) override;
};

}}}
