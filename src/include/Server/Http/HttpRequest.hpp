#pragma once

#include <string>
#include <vector>
#include <tuple>

namespace webcrown {
namespace server {
namespace http {

class HttpRequest
{

  ///
  std::string buffer_;
public:

  HttpRequest(HttpRequest const&) = default;
  HttpRequest(HttpRequest&&) = default;

  HttpRequest& operator=(HttpRequest const&) = default;
  HttpRequest& operator=(HttpRequest&&) = default;

  ~HttpRequest() = default;


  void parse(void const* const buffer, size_t size);
};

}}}