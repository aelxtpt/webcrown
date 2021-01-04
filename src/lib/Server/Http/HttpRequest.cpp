#include "Server/Http/HttpRequest.hpp"

namespace webcrown {
namespace server {
namespace http {

void HttpRequest::parse(void const* const buffer, size_t size)
{
  buffer_.insert(buffer_.end(), (const char*)buffer, (const char*)buffer + size);
}

}}}