#pragma once

#include <string>
#include <vector>
#include <tuple>

namespace webcrown {
namespace http {

class HttpRequest
{
  // HTTP request error flag
  bool error_;
  // HTTP request method
  size_t method_index_;
  size_t method_size_;
  // HTTP request URL
  size_t url_index_;
  size_t url_size_;
  // HTTP request protocol
  size_t protocol_index_;
  size_t protocol_size_;
  // HTTP request headers
  std::vector<std::tuple<size_t, size_t, size_t, size_t>> headers_;
  // HTTP request cookies
  std::vector<std::tuple<size_t, size_t, size_t, size_t>> cookies_;
  // HTTP request body
  size_t body_index_;
  size_t body_size_;
  size_t body_length_;
  bool body_length_provided_;

  // HTTP request cache
  std::string cache_;
  size_t cache_size_;

  // Is pending parts of HTTP response
  bool is_pending_header() const;
  bool is_pending_body() const;

  // Receive parts of HTTP response
  bool receive_header(void const* buffer, size_t size);
  bool receive_body(void const* buffer, size_t size);

  // Fast convert integer value to the corresponding string representation
  std::string_view fast_convert(size_t value, char* buffer, size_t size);

public:

  explicit HttpRequest(
    std::string_view method,
    std::string_view url,
    std::string_view protocol = "HTTP/1.1"
  );

  HttpRequest(HttpRequest const&) = default;
  HttpRequest(HttpRequest&&) = default;

  HttpRequest& operator=(HttpRequest const&) = default;
  HttpRequest& operator=(HttpRequest&&) = default;

  ~HttpRequest() = default;

  /// Verify if the HTTP request is empty
  /// 
  /// \return true if the http request is empty
  bool empty() const noexcept { return cache_.empty(); }

  /// Verify if exists an error on the HTTP Request
  /// 
  /// \return true if the error exists
  bool error() const noexcept { return error_; }

  /// Get the HTTP request method
  std::string_view method() const noexcept 
  { return std::string_view(cache_.data() + method_index_, method_size_); }

  /// Get the HTTP request url
  std::string_view url() const noexcept 
  { return std::string_view(cache_.data() + url_index_, url_size_); }

  /// Get the HTTP request protocol
  std::string_view protocol() const noexcept
  { return std::string_view(cache_.data() + protocol_index_, protocol_size_); }

  /// Get the HTTP request headers count
  std::size_t headers_count() const noexcept
  { return headers_.size(); }

  /// Clear the HTTP request cache
  HttpRequest& clear();

  /// Set the HTTP request begin with a given method,
  /// URL and protocol
  /// 
  /// \param method - Http method
  /// \param url - Requested url
  /// \param protocol - Protocol Version (default is "HTTP/1.1")
  HttpRequest& 
  set_begin(std::string_view method, std::string_view url, std::string_view protocol = "HTTP/1.1");

  /// Set the HTTP request header
  /// 
  /// \param key - Header key
  /// \param value - Header value
  HttpRequest&
  set_header(std::string_view key, std::string_view value);

  /// Set the HTTP request cookie
  /// 
  /// \param name - Cookie name
  /// \param value - Cookie value
  HttpRequest&
  set_cookie(std::string_view name, std::string_view value);

  /// Add the HTTP request cookie
  /// 
  /// \param name 
  /// \param value 
  /// \return HttpRequest& 
  HttpRequest&
  add_cookie(std::string_view name, std::string_view value);

  /// Set the HTTP request body
  /// 
  /// \param body - Body content (default is "")
  HttpRequest&
  set_body(std::string_view body = "");

  /// Set the HTTP reqyest body length
  /// 
  /// \param length - Body length
  HttpRequest&
  set_body_length(size_t length);

  /// Make HEAD request
  /// 
  /// \param url - URL to request
  HttpRequest& 
  make_head_request(std::string_view url);

  /// Make GET request
  /// 
  /// \param url - URl to request
  HttpRequest&
  make_get_request(std::string_view url);

  /// Make POST request
  /// 
  /// \param url - URL to request
  /// \param content - Content
  /// \param content_type - Content Type (default is "text/plain; charset=UTF-8")
  HttpRequest&
  make_post_request(std::string_view url, std::string_view content, std::string_view content_type = "text/plain; charset=UTF-8");

  /// Make PUT request
  /// 
  /// \param url - URL to request
  /// \param content - Content
  /// \param content_type - Content type (default is "text/plain; charset=UTF-8")
  HttpRequest&
  make_put_request(std::string_view url, std::string_view content, std::string_view content_type = "text/plain; charset=UTF-8");

  /// Make DELETE request
  /// 
  /// \param url - URL to request
  HttpRequest&
  make_delete_request(std::string_view url);

  /// Make OPTIONS request
  /// 
  /// \param url - URL to request
  HttpRequest&
  make_options_request(std::string_view url);

  /// Make TRACE request
  /// 
  /// \param url - URL to request
  HttpRequest&
  make_trace_request(std::string_view url);

  /// Output instance into given output stream
  /// 
  /// \param os 
  /// \param request 
  //friend std::ostream& operator<<(std::ostream& os, HttpRequest const& request);


};

}}