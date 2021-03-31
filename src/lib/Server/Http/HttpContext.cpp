#include "Server/Http/HttpContext.hpp"
#include "Server/Http/http_session.hpp"
#include "Server/Http/HttpRequest.hpp"
#include "Server/Http/HttpEndpoint.hpp"

namespace webcrown {
namespace server {
namespace http {

HttpContext::HttpContext(std::shared_ptr<http_session> session)
  : session_(session)
{

}

void HttpContext::add_endpoint(std::shared_ptr<HttpEndpoint> const endpoint)
{
  endpoints_.push_back(endpoint);
}

}}}
