#pragma once

#include "webcrown/server/http/method.hpp"

namespace webcrown {
namespace server {
namespace http {

class http_request
{
    method method_;
    int protocol_;
public:

    method method() const noexcept { return method_; }
};

}}}