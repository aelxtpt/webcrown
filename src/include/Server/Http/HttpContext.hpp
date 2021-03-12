#pragma once

#include <memory>
#include <functional>
#include <tuple>
#include "Server/Http/HttpRequest.hpp"

namespace webcrown {
namespace server {
namespace http {

class HttpSession;
class HttpRequest;
class HttpEndpoint;

class HttpContext
{
  std::shared_ptr<HttpSession> session_;

  std::vector<std::shared_ptr<HttpEndpoint>> endpoints_;
public:
  HttpContext(std::shared_ptr<HttpSession> session);

  void add_endpoint(std::shared_ptr<HttpEndpoint> const endpoint);

  std::vector<std::shared_ptr<HttpEndpoint>> endpoints() const noexcept { return endpoints_; }
};

}}}
