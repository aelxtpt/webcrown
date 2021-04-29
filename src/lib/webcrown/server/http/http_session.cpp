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
    std::shared_ptr<spdlog::logger> const& logger,
    std::shared_ptr<asio::ssl::context> const& context)
  : session(session_id, server, logger, context)
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
        // error
        return;
    }

    http_response response{};
    // middlewares
    for(auto& middleware : middlewares_)
    {
        middleware->on_setup(*result, response);
    }
    //response.set_status(http_status::ok);
    //response.set_body("ola");

    // send response
    send_async(response.build());
    logger_->info("Mensagem enviada para o client");

    // disconnect 
    //disconnect();
}

}}}
