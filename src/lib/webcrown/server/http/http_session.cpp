#include "webcrown/server/http/http_session.hpp"
#include "webcrown/server/http/http_server.hpp"
#include "webcrown/server/http/parser.hpp"

#include <algorithm>

namespace webcrown {
namespace server {
namespace http {

http_session::http_session(
    uint64_t session_id,
    std::shared_ptr<http_server> server,
    std::shared_ptr<spdlog::logger> const& logger,
    std::shared_ptr<asio::ssl::context> const& context)
  : session(session_id, server, logger, context)
{
}

void http_session::on_received(void const* buffer, std::size_t size)
{
    std::error_code ec{};

    // parser
    parser p;
    auto result = p.parse_start_line(static_cast<const char*>(buffer), size, ec);
    if (!result)
    {
        // error
        return;
    }

    http_response response{};
    // middlewares
    for(auto const& middleware : middlewares_)
    {
        middleware->on_setup(*result, response);
    }

    // send response

}

}}}
