#pragma once

#include <memory>

#include "webcrown/server/http/middlewares/http_middleware.hpp"

namespace webcrown {
namespace server {
namespace http {

class cors_middleware : public middleware
{
public:
    explicit cors_middleware()
    {}

    bool execute(http_request const& request, http_response& response, std::shared_ptr<spdlog::logger> logger) override
    {
        auto&& headers = request.headers();
        auto origin = headers.find("Origin");

        if (origin == headers.end())
        {
            SPDLOG_LOGGER_DEBUG(logger, "webcrown::cors_middleware::on_setup Header Origin not found.");
            return true;
        }

        auto host = headers.find("Host");
        if (host == headers.end())
        {
            // not found required header
            SPDLOG_LOGGER_DEBUG(logger, "webcrown::cors_middleware::on_setup Header Host not found.");
            return false;
        }

        if(origin->second == ("http://"+ host->second) || origin->second == ("https://"+host->second))
        {
            // request is not a CORS request but have origin header.
            // for example, use fetch api
            return false;
        }

        // TODO: Apply some validations

        // Apply cors
        response.add_header("Access-Control-Allow-Origin", "*");
        response.add_header("Access-Control-Allow-Headers", "*");
        response.add_header("Access-Control-Allow-Methods", "*");

        if (request.method() == http_method::options)
        {
            response.set_status(http_status::no_content);
            return false;
        }

        return true;
    }
};


}}}
