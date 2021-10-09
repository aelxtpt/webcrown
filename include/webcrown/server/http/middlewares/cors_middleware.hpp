#pragma once

#include <memory>
#include <spdlog/logger.h>
#include "webcrown/server/http/middlewares/http_middleware.hpp"

namespace webcrown {
namespace server {
namespace http {

class cors_middleware : public middleware
{
    bool should_return_now_;
public:
    explicit cors_middleware()
        : should_return_now_(false)
    {}

    void on_setup(http_request const& request, http_response& response) override
    {
        auto&& headers = request.headers();
        auto origin = headers.find("Origin");

        if (origin == headers.end())
        {
            // not found required header
            return;
        }

        auto host = headers.find("Host");
        if (host == headers.end())
        {
            // not found required header
            return;
        }

        if(origin->second == ("http://"+ host->second) || origin->second == ("https://"+host->second))
        {
            // request is not a CORS request but have origin header.
            // for example, use fetch api
            return;
        }

        // TODO: Apply some validations

        // Apply cors
        response.add_header("Access-Control-Allow-Origin", "*");
        response.add_header("Access-Control-Allow-Headers", "*");

        if (request.method() == http_method::options)
        {
            response.set_status(http_status::no_content);
            should_return_now_ = true;
        }
    }

    bool should_return_now() override
    {
        return should_return_now_;
    }

    void should_return_now(bool flag) override
    {
        should_return_now_ = flag;
    }
};


}}}
