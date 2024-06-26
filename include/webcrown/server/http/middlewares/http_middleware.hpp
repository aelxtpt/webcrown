#pragma once
#include <deque>
#include "webcrown/server/http/http_request.hpp"
#include "webcrown/server/http/http_response.hpp"

namespace webcrown {
namespace server {
namespace http {

struct my_auth_options
{
    int xpto1;
    int xpto2;
};

class middleware
{
public:
    virtual ~middleware() = default;

    virtual bool execute(http_request const& request, http_response& response) = 0;
};

}}}
