#include "Server/Http/HttpEndpoint.hpp"
#include "Server/Http/HttpRequest.hpp"

namespace webcrown {
namespace server {
namespace http {

HttpEndpoint::HttpEndpoint(
    std::string const& method,
    std::string const& uri,
    std::function<void(HttpRequest const&)> const& callback)
  : method_(method)
  , uri_(uri)
  , callback_(callback)
{}

}}}
