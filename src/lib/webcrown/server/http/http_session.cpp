#include "webcrown/server/http/http_session.hpp"
#include "webcrown/server/http/http_server.hpp"
#include "webcrown/server/http/http_parser.hpp"

#include <algorithm>

namespace webcrown {
namespace server {
namespace http {

http_session::http_session(
    uint64_t session_id,
    std::shared_ptr<http_server> server,
    std::shared_ptr<spdlog::logger> const& logger)
  : session(session_id, server, logger)
  , logger_(logger)
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
        logger_->error("[http_session][on_received] failed to parser start line {}", ec.message());
        return;
    }

    http_response response{};
    // middlewares
    for(auto& middleware : middlewares_)
    {
        middleware->on_setup(*result, response);
        if (middleware->should_return_now())
        {
            // TODO: GAMBI 2. Muito ruim, se esquecer, quebra tudo
            middleware->should_return_now(false);
            break;
        }
    }

    // send response
    send_async(response.build());
    logger_->info("[http_session][on_received] Message sent to the client");

    // disconnect 
    disconnect();
}

}}}
