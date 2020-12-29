#include "Server/Http/HttpRequest.hpp"

namespace webcrown {
namespace http {

HttpRequest& HttpRequest::clear()
{
  error_ = false;
  method_index_ = 0;
  method_size_ = 0;
  url_index_ = 0;
  url_size_ = 0;
  protocol_index_ = 0;
  protocol_size_ = 0;
  headers_.clear();
  cookies_.clear();
  body_index_ = 0;
  body_size_ = 0;
  body_length_ = 0;
  body_length_provided_ = false;

  cache_.clear();
  cache_size_ = 0;

  return *this;
}

HttpRequest& 
HttpRequest::set_begin(std::string_view method, std::string_view url, std::string_view protocol)
{
  // Clear the HTTP request cache
  clear();

  size_t index = 0;

  // Append the Http Request method

  return *this;
}

HttpRequest&
HttpRequest::set_header(std::string_view key, std::string_view value)
{
  return *this;
}

HttpRequest&
HttpRequest::set_cookie(std::string_view name, std::string_view value)
{
  return *this;
}

HttpRequest&
HttpRequest::add_cookie(std::string_view name, std::string_view value)
{
  return *this;
}

HttpRequest&
HttpRequest::set_body(std::string_view body)
{
  return *this;
}

HttpRequest&
HttpRequest::set_body_length(size_t length)
{
  return *this;
}

HttpRequest&
HttpRequest::make_head_request(std::string_view url)
{
  return *this;
}

HttpRequest&
HttpRequest::make_get_request(std::string_view url)
{
  return *this;
}

HttpRequest&
HttpRequest::make_post_request(std::string_view url, std::string_view content, std::string_view content_type)
{
  return *this;
}

HttpRequest&
HttpRequest::make_put_request(std::string_view url, std::string_view content, std::string_view content_type)
{
  return *this;
}

HttpRequest&
HttpRequest::make_delete_request(std::string_view url)
{
  return *this;
}

HttpRequest&
HttpRequest::make_options_request(std::string_view url)
{
  return *this;
}

HttpRequest&
HttpRequest::make_trace_request(std::string_view url)
{
  return *this;
}

}}