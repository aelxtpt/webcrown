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

    bool execute(http_request const& request, http_response& response) override
    {
        auto&& headers = request.headers();
        auto origin = headers.find("origin");

        if (origin == headers.end())
        {
            return true;
        }

        auto host = headers.find("host");
        if (host == headers.end())
        {
            // not found required header
            return false;
        }

        if(origin->second == ("http://"+ host->second) || origin->second == ("https://"+host->second))
        {
            // request is not a CORS request but have origin header.
            // for example, use fetch api
            return true;
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
