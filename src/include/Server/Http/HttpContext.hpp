#pragma once

#include <memory>
#include <functional>
#include <tuple>
#include "Server/Http/HttpRequest.hpp"

namespace webcrown {
namespace server {
namespace http {

class http_session;
class HttpRequest;
class HttpEndpoint;

class HttpContext
{
  std::shared_ptr<http_session> session_;

  std::vector<std::shared_ptr<HttpEndpoint>> endpoints_;
public:
  HttpContext(std::shared_ptr<http_session> session);

  void add_endpoint(std::shared_ptr<HttpEndpoint> const endpoint);

  std::vector<std::shared_ptr<HttpEndpoint>> endpoints() const noexcept { return endpoints_; }
};

}}}
