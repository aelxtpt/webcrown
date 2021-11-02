#pragma once
#include "webcrown/server/session.hpp"
#include "webcrown/server/http/middlewares/http_middleware.hpp"
#include "webcrown/server/http/http_parser.hpp"

namespace webcrown {
namespace server {
namespace http {

class http_server;

class http_session :
	public webcrown::server::session,
	public std::enable_shared_from_this<http_session>
{
    std::vector<std::shared_ptr<middleware>> middlewares_;
    parser parser_;
    std::shared_ptr<spdlog::logger> logger_;
public:
    explicit http_session(
      uint64_t session_id,
      std::shared_ptr<http_server> server);

    http_session(http_session const&) = delete;
    http_session(http_session &&) = delete;

    http_session& operator=(http_session const&) = delete;
    http_session& operator=(http_session&&) = delete;

    ~http_session() = default;

    void middlewares(std::vector<std::shared_ptr<middleware>> const& middlewares) noexcept
    { middlewares_ = middlewares; }

    void on_received(void const* buffer, std::size_t size) override;

    // session interface

    // session interface
private:
    void on_error(asio::error_code &ec) override;
};

}}}
