#pragma once
#include "webcrown/server/http/middlewares/http_middleware.hpp"

namespace webcrown {
namespace server {
namespace http {

class routing_middleware : public middleware
{
public:
    void on_setup(http_request const& request, http_response& response) override
    {

    }

public:

};

}}}