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
    logger_ = session::logger();
    parser_ = parser(logger_);
}

void http_session::on_received(void const* buffer, std::size_t size)
{
    std::error_code ec{};

    SPDLOG_LOGGER_DEBUG(logger_, "webcrown::http_session::on_received **socket level, no parse** buffer_size: {} buffer: {}",
                        size,
                        static_cast<const char*>(buffer)
    );

    // parser
    auto result = parser_.parse(static_cast<const char*>(buffer), size, ec);
    if (ec)
    {
        SPDLOG_LOGGER_DEBUG(logger_, "webcrown::http_session::on_received Failed to parse start line {}",
                            ec.message());
        //logger_->error("[http_session][on_received] failed to parser start line {}", ec.message());
        return;
    }

    if (parser_.parsephase() != parse_phase::finished)
    {
        // need more
        //logger_->info("[http_session][on_received] need more bytes...");
        SPDLOG_LOGGER_DEBUG(logger_, "webcrown::http_session::on_received Need more bytes...");
        return;
    }

    if (!result)
    {
        //logger_->error("[http_session][on_received] no http request");
        SPDLOG_LOGGER_DEBUG(logger_, "webcrown::http_session::on_received No http request");
        return;
    }

    http_response response{};
    // middlewares
    for(auto& middleware : middlewares_)
    {
        middleware->on_setup(*result, response, logger_);
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
        SPDLOG_LOGGER_DEBUG(logger_, "webcrown::http_session::on_received Failed to disconnect session");
        //logger_->error("[http_session][on_received] Failed to disconnect session.");
    }
}

void http_session::on_error(asio::error_code &ec)
{

}

}}}
