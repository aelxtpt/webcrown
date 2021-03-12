#pragma once

#include <string>
#include <vector>
#include <tuple>
#include "details/enums.hpp"

namespace webcrown {
namespace server {
namespace http {

class HttpRequest
{
  friend class HttpSession;

  // HTTP request error flag
  bool error_;

  // HTTP Request method type
  HttpMethod method_type_;

  // HTTP request method
  std::size_t method_index_;
  std::size_t method_size_;

  // HTTP request URL
  std::size_t url_index_;
  std::size_t url_size_;

  // HTTP request protocol
  std::size_t protocol_index_;
  std::size_t protocol_size_;

  // HTTP Request Headers
  std::vector<std::tuple<std::size_t, std::size_t, std::size_t, std::size_t>> headers_;

  // HTTP request cookies
  std::vector<std::tuple<std::size_t, std::size_t, std::size_t, std::size_t>> cookies_;

  // HTTP request body
  std::size_t body_index_;
  std::size_t body_size_;
  std::size_t body_length_;
  bool body_length_provided_;

  // HTTP request cache
  std::string cache_;
  std::size_t cache_size_;

  // Receive parts of HTTP response
  bool is_pending_header() const;
  bool is_pending_body() const;

  // Receive parts of HTTP response
  bool receive_header(void const* buffer, std::size_t size);
  bool receive_body(void const* buffer, std::size_t size);

  //std::string_view fast_converter

  std::string buffer_;
public:

  //! Initialize an empty HTTP Request
  HttpRequest() { clear(); }

  HttpRequest(HttpRequest const&) = default;
  HttpRequest(HttpRequest&&) = default;

  HttpRequest& operator=(HttpRequest const&) = default;
  HttpRequest& operator=(HttpRequest&&) = default;

  ~HttpRequest() = default;


  void parse(void const* const buffer, std::size_t size);

  //! Is the HTTP request error flag set?
  bool error() const noexcept { return error_; }


  //! Get the HTTP request method
  std::string_view method() const noexcept { return std::string_view(cache_.data() + method_index_, method_size_); }

  //! Get the HTTP request URL
  std::string_view url() const noexcept { return std::string_view(cache_.data() + url_index_, url_size_); }
  //! Get the HTTP request protocol version
  std::string_view protocol() const noexcept { return std::string_view(cache_.data() + protocol_index_, protocol_size_); }
  //! Get the HTTP request headers count
  std::size_t headers() const noexcept { return headers_.size(); }
  //! Get the HTTP request cookies count
  std::size_t cookies() const noexcept { return cookies_.size(); }
  //! Get the HTTP request body
  std::string_view body() const noexcept { return std::string_view(cache_.data() + body_index_, body_size_); }
  //! Get the HTTP request body length
  std::size_t body_length() const noexcept { return body_length_; }

  // Clear the HTTP request cache
  HttpRequest& clear() noexcept;

  //! Set the HTTP Request begin with a given method, URL and protocol
  /*!
  * \brief set_begin
  * \param method - Http Method
  * \param url - Requested URL
  * \param protocol - Protocol Version (default is HTTP/1.1)
  * \return
  */
  HttpRequest& set_begin(std::string_view method, std::string_view url, std::string_view protocol = "HTTP/1.1");

  //! Set the HTTP request cookie
  /*!
   * \brief set_cookie
   * \param name - Cookie name
   * \param value - Cookie value
   * \return
   */
  HttpRequest& set_cookie(std::string_view name, std::string_view value);

  //! Add the HTTP Request cookie
  /*!
   * \brief add_cookie
   * \param name - Cookie name
   * \param value - Cookie value
   * \return
   */
  HttpRequest& add_cookie(std::string_view name, std::string_view value);

  //! Set the HTTP request body
  /*!
   * \brief set_body
   * \param body - Body content (default is "")
   * \return
   */
  HttpRequest& set_body(std::string_view body = "");

  //! Set the HTTP request Header
  HttpRequest& set_header(std::string_view key, std::string_view value);

  //! Set the HTTP Request body length
  /*!
   * \brief set_body_length
   * \param length - body length
   * \return
   */
  HttpRequest& set_body_length(std::size_t length);

  //! Make HEAD request
  /*!
   * \brief make_head_request
   * \param url - URL to request
   * \return
   */
  HttpRequest& make_head_request(std::string_view url);

  //! Make GET request
  /*!
   * \brief make_get_request
   * \param url - URL to request
   * \return
   */
  HttpRequest& make_get_request(std::string_view url);

  //! Make POST request
  /*!
   * \brief make_post_request
   * \param url - URL to request
   * \param content - Content
   * \param content_type - Content type (default is "text/plain; charset=UTF-8")
   * \return
   */
  HttpRequest& make_post_request(std::string_view url, std::string_view content, std::string_view content_type = "text/plain; charset=UTF-8");

  //! Make PUT request
  /*!
   * \brief make_put_request
   * \param url - URL to request
   * \param content - Content
   * \param content_type - Content type (default is "text/plain; charset=UTF-8")
   * \return
   */
  HttpRequest& make_put_request(std::string_view url, std::string_view content, std::string_view content_type = "text/plain; charset=UTF-8");

  //! Make DELETE request
  /*!
   * \brief make_delete_request
   * \param url - URL to request
   * \return HTTP Request
   */
  HttpRequest& make_delete_request(std::string_view url);

  //! Make OPTIONS request
  /*!
   * \brief make_options_request
   * \param url - URL to request
   * \return HTTP Request
   */
  HttpRequest& make_options_request(std::string_view url);

  // Make TRACE request
  /*!
   * \brief make_trace_request
   * \param url - URL to request
   * \return HTTP Request
   */
  HttpRequest& make_trace_request(std::string_view url);

  // Get the HTTP request header by index
  std::tuple<std::string_view, std::string_view> header(std::size_t i) const noexcept;

  // Get the HTTP request cookie by index
  std::tuple<std::string_view, std::string_view> cookie(std::size_t i) const noexcept;

  //! Fast convert integer value to the corresponding string representation
  std::string_view fast_convert(std::size_t value, char* buffer, std::size_t size);

  //! Output instance into the given output stream
  friend std::ostream& operator<<(std::ostream& os, const HttpRequest& request);

  //! Swap two instances
  void swap(HttpRequest& request) noexcept;
  friend void swap(HttpRequest& request1, HttpRequest& request2) noexcept { request1.swap(request2); }


private:


};

}}}
