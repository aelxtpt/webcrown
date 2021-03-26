#pragma once

#include <algorithm>
#include <string>

namespace webcrown {
namespace server {
namespace http {

class HttpRequest;

class HttpEndpoint
{
  std::string method_;
  std::string uri_;
  std::function<void(HttpRequest const&)> callback_;
public:
  explicit HttpEndpoint(
      std::string const& method,
      std::string const& uri,
      std::function<void(HttpRequest const&)> const& callback);

  std::string const& uri() const noexcept { return uri_; }

  std::string method() const noexcept { return method_; }

  std::function<void(HttpRequest const&)> const callback() const noexcept { return callback_; }
};

}}}
