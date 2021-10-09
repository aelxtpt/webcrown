#include "webcrown/server/http/http_session.hpp"
#include "webcrown/server/http/http_server.hpp"


#include <algorithm>

namespace webcrown {
namespace server {
namespace http {

http_session::http_session(uint64_t session_id,
    std::shared_ptr<http_server> server)
  : session(session_id, server)
{
}

void http_session::on_received(void const* buffer, std::size_t size)
{
    std::error_code ec{};

    // parser
    auto result = parser_.parse(static_cast<const char*>(buffer), size, ec);
    if (ec)
    {
        //logger_->error("[http_session][on_received] failed to parser start line {}", ec.message());
        return;
    }

    if (parser_.parsephase() != parse_phase::finished)
    {
        // need more
        //logger_->info("[http_session][on_received] need more bytes...");
        return;
    }

    if (!result)
    {
        //logger_->error("[http_session][on_received] no http request");
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
    //logger_->info("[http_session][on_received] Message sent to the client");

    // disconnect 
    if(!disconnect())
    {
        //logger_->error("[http_session][on_received] Failed to disconnect session.");
    }
}

void http_session::on_error(asio::error_code &ec)
{

}

}}}
