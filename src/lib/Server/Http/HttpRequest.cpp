#include "Server/Http/HttpRequest.hpp"
#include <cassert>
#include <iostream>

namespace webcrown {
namespace server {
namespace http {

//! Count of elements in static array
template <typename T, std::size_t N>
constexpr std::size_t countof(const T (&)[N]) noexcept { return N; }

//! Count of elements in any other STL container
template <typename T>
std::size_t countof(const T& container) noexcept { return container.size(); }

void HttpRequest::parse(void const* const buffer, std::size_t size)
{
  buffer_.insert(buffer_.end(), (const char*)buffer, (const char*)buffer + size);


}

HttpRequest& HttpRequest::clear() noexcept
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

HttpRequest& HttpRequest::set_begin(std::string_view method, std::string_view url, std::string_view protocol)
{
  // Clear the HTTP request cache
  clear();

  std::size_t index = 0;

  // Append the HTTP request method
  cache_.append(method);
  method_index_ = index;
  method_size_ = method.size();

  cache_.append(" ");
  index = cache_.size();

  // Append the HTTP request protocol version
  cache_.append(protocol);
  protocol_index_ = index;
  protocol_size_ = protocol.size();

  cache_.append("\r\n");

  return *this;
}

HttpRequest& HttpRequest::set_cookie(std::string_view name, std::string_view value)
{
  std::size_t index = cache_.size();

  // Append the HTTP request header's key
  cache_.append("Cookie");
  std::size_t key_index = index;
  std::size_t key_size = 6;

  cache_.append(": ");
  index = cache_.size();

  // Append the HTTP request header' value
  std::size_t value_index = index;

  // Append Cookie
  index = cache_.size();
  cache_.append(name);
  std::size_t name_index = index;
  std::size_t name_size = name.size();

  cache_.append("=");
  index = cache_.size();
  cache_.append(value);
  std::size_t cookie_index = index;
  std::size_t cookie_size = value.size();

  std::size_t value_size = cache_.size() - value_index;

  cache_.append("\r\n");

  // Add header to the corresponding collection
  headers_.emplace_back(key_index, key_size, value_index, value_size);

  // Add the cookie to the corresponding collection
  cookies_.emplace_back(name_index, name_size, cookie_index, cookie_size);
  return *this;
}

HttpRequest& HttpRequest::add_cookie(std::string_view name, std::string_view value)
{
  // Append Cookie
  cache_.append("; ");
  std::size_t index = cache_.size();
  cache_.append(name);
  std::size_t name_index = index;
  std::size_t name_size = name.size();
  cache_.append("=");
  index = cache_.size();
  cache_.append(value);
  std::size_t cookie_index = index;
  std::size_t cookie_size = value.size();

  // Add the cookie to the corresponding collection
  cookies_.emplace_back(name_index, name_size, cookie_index, cookie_size);
  return *this;
}

HttpRequest& HttpRequest::set_body(std::string_view body)
{
  // Append content length header
  char buffer[32];
  set_header("Content-Length", fast_convert(body.size(), buffer, countof(buffer)));

  cache_.append("\r\n");

  std::size_t index = cache_.size();

  // Append the HTTP request body
  cache_.append(body);
  body_index_ = index;
  body_size_ = body.size();
  body_length_ = body.size();
  body_length_provided_ = true;
  return *this;

}

HttpRequest& HttpRequest::set_header(std::string_view key, std::string_view value)
{
  std::size_t index = cache_.size();

  // Append the HTTP request header's key
  cache_.append(key);
  std::size_t key_index = index;
  std::size_t key_size = key.size();

  cache_.append(": ");
  index = cache_.size();

  // Append the HTTP request header's value
  cache_.append(value);
  std::size_t value_index = index;
  std::size_t value_size = value.size();

  cache_.append("\r\n");

  // Add the header to the corresponding collection
  headers_.emplace_back(key_index, key_size, value_index, value_size);
  return *this;
}

HttpRequest& HttpRequest::set_body_length(std::size_t length)
{
    // Append content length header
    char buffer[32];
    set_header("Content-Length", fast_convert(length, buffer, countof(buffer)));

    cache_.append("\r\n");

    std::size_t index = cache_.size();

    // Clear the HTTP request body
    body_index_ = index;
    body_size_ = 0;
    body_length_ = length;
    body_length_provided_ = true;
    return *this;
}

HttpRequest& HttpRequest::make_head_request(std::string_view url)
{
    clear();
    set_begin("HEAD", url);
    set_body();
    return *this;
}

HttpRequest& HttpRequest::make_get_request(std::string_view url)
{
    clear();
    set_begin("GET", url);
    set_body();
    return *this;
}

HttpRequest& HttpRequest::make_post_request(std::string_view url, std::string_view content, std::string_view content_type)
{
    clear();
    set_begin("POST", url);
    if (!content_type.empty())
        set_header("Content-Type", content_type);
    set_body(content);
    return *this;
}

HttpRequest& HttpRequest::make_put_request(std::string_view url, std::string_view content, std::string_view content_type)
{
    clear();
    set_begin("PUT", url);
    if (!content_type.empty())
        set_header("Content-Type", content_type);
    set_body(content);
    return *this;
}

HttpRequest& HttpRequest::make_delete_request(std::string_view url)
{
    clear();
    set_begin("DELETE", url);
    set_body();
    return *this;
}

HttpRequest& HttpRequest::make_options_request(std::string_view url)
{
    clear();
    set_begin("OPTIONS", url);
    set_body();
    return *this;
}

HttpRequest& HttpRequest::make_trace_request(std::string_view url)
{
    clear();
    set_begin("TRACE", url);
    set_body();
    return *this;
}

bool HttpRequest::is_pending_header() const
{
    return (!error_ && (body_index_ == 0));
}

bool HttpRequest::is_pending_body() const
{
    return (!error_ && (body_index_ > 0) && (body_size_ > 0));
}

bool HttpRequest::receive_header(const void* buffer, std::size_t size)
{
    // Update the request cache
    cache_.insert(cache_.end(), (const char*)buffer, (const char*)buffer + size);

    // Try to seek for HTTP header separator
    for (std::size_t i = cache_size_; i < cache_.size(); ++i)
    {
        // Check for the request cache out of bounds
        if ((i + 3) >= cache_.size())
            break;

        // Check for the header separator
        if ((cache_[i + 0] == '\r') && (cache_[i + 1] == '\n') && (cache_[i + 2] == '\r') && (cache_[i + 3] == '\n'))
        {
            std::size_t index = 0;

            // Set the error flag for a while...
            error_ = true;

            // Parse method
            method_index_ = index;
            method_size_ = 0;
            while (cache_[index] != ' ')
            {
                ++method_size_;
                ++index;
                if (index >= cache_.size())
                    return false;
            }
            ++index;
            if (index >= cache_.size())
                return false;

            // Parse URL
            url_index_ = index;
            url_size_ = 0;
            while (cache_[index] != ' ')
            {
                ++url_size_;
                ++index;
                if (index >= cache_.size())
                    return false;
            }
            ++index;
            if (index >= cache_.size())
                return false;

            // Parse protocol version
            protocol_index_ = index;
            protocol_size_ = 0;
            while (cache_[index] != '\r')
            {
                ++protocol_size_;
                ++index;
                if (index >= cache_.size())
                    return false;
            }
            ++index;
            if ((index >= cache_.size()) || (cache_[index] != '\n'))
                return false;
            ++index;
            if (index >= cache_.size())
                return false;

            // Parse headers
            while ((index < cache_.size()) && (index < i))
            {
                // Parse header name
                std::size_t header_name_index = index;
                std::size_t header_name_size = 0;
                while (cache_[index] != ':')
                {
                    ++header_name_size;
                    ++index;
                    if (index >= i)
                        break;
                    if (index >= cache_.size())
                        return false;
                }
                ++index;
                if (index >= i)
                    break;
                if (index >= cache_.size())
                    return false;

                // Skip all prefix space characters
                while (std::isspace(cache_[index]))
                {
                    ++index;
                    if (index >= i)
                        break;
                    if (index >= cache_.size())
                        return false;
                }

                // Parse header value
                std::size_t header_value_index = index;
                std::size_t header_value_size = 0;
                while (cache_[index] != '\r')
                {
                    ++header_value_size;
                    ++index;
                    if (index >= i)
                        break;
                    if (index >= cache_.size())
                        return false;
                }
                ++index;
                if ((index >= cache_.size()) || (cache_[index] != '\n'))
                    return false;
                ++index;
                if (index >= cache_.size())
                    return false;

                // Validate header name and value
                if ((header_name_size == 0) || (header_value_size == 0))
                    return false;

                // Add a new header
                headers_.emplace_back(header_name_index, header_name_size, header_value_index, header_value_size);

                // Try to find the body content length
                if (std::string_view(cache_.data() + header_name_index, header_name_size) == "Content-Length")
                {
                    body_length_ = 0;
                    for (std::size_t j = header_value_index; j < (header_value_index + header_value_size); ++j)
                    {
                        if ((cache_[j] < '0') || (cache_[j] > '9'))
                            return false;
                        body_length_ *= 10;
                        body_length_ += cache_[j] - '0';
                        body_length_provided_ = true;
                    }
                }

                // Try to find Cookies
                if (std::string_view(cache_.data() + header_name_index, header_name_size) == "Cookie")
                {
                    bool name = true;
                    bool token = false;
                    std::size_t current = header_value_index;
                    std::size_t name_index = index;
                    std::size_t name_size = 0;
                    std::size_t cookie_index = index;
                    std::size_t cookie_size = 0;
                    for (std::size_t j = header_value_index; j < (header_value_index + header_value_size); ++j)
                    {
                        if (cache_[j] == ' ')
                        {
                            if (token)
                            {
                                if (name)
                                {
                                    name_index = current;
                                    name_size = j - current;
                                }
                                else
                                {
                                    cookie_index = current;
                                    cookie_size = j - current;
                                }
                            }
                            token = false;
                            continue;
                        }
                        if (cache_[j] == '=')
                        {
                            if (token)
                            {
                                if (name)
                                {
                                    name_index = current;
                                    name_size = j - current;
                                }
                                else
                                {
                                    cookie_index = current;
                                    cookie_size = j - current;
                                }
                            }
                            token = false;
                            name = false;
                            continue;
                        }
                        if (cache_[j] == ';')
                        {
                            if (token)
                            {
                                if (name)
                                {
                                    name_index = current;
                                    name_size = j - current;
                                }
                                else
                                {
                                    cookie_index = current;
                                    cookie_size = j - current;
                                }

                                // Validate the cookie
                                if ((name_size > 0) && (cookie_size > 0))
                                {
                                    // Add the cookie to the corresponding collection
                                    cookies_.emplace_back(name_index, name_size, cookie_index, cookie_size);

                                    // Resset the current cookie values
                                    name_index = j;
                                    name_size = 0;
                                    cookie_index = j;
                                    cookie_size = 0;
                                }
                            }
                            token = false;
                            name = true;
                            continue;
                        }
                        if (!token)
                        {
                            current = j;
                            token = true;
                        }
                    }

                    // Process the last cookie
                    if (token)
                    {
                        if (name)
                        {
                            name_index = current;
                            name_size = header_value_index + header_value_size - current;
                        }
                        else
                        {
                            cookie_index = current;
                            cookie_size = header_value_index + header_value_size - current;
                        }

                        // Validate the cookie
                        if ((name_size > 0) && (cookie_size > 0))
                        {
                            // Add the cookie to the corresponding collection
                            cookies_.emplace_back(name_index, name_size, cookie_index, cookie_size);
                        }
                    }
                }
            }

            // Reset the error flag
            error_ = false;

            // Update the body index and size
            body_index_ = i + 4;
            body_size_ = cache_.size() - i - 4;

            // Update the parsed cache size
            cache_size_ = cache_.size();

            return true;
        }
    }

    // Update the parsed cache size
    cache_size_ = (cache_.size() >= 3) ? (cache_.size() - 3) : 0;

    return false;
}

bool HttpRequest::receive_body(const void* buffer, std::size_t size)
{
    // Update HTTP request cache
    cache_.insert(cache_.end(), (const char*)buffer, (const char*)buffer + size);

    // Update the parsed cache size
    cache_size_ = cache_.size();

    // Update body size
    body_size_ += size;

    // GET request has no body
    if ((method() == "HEAD") || (method() == "GET") || (method() == "OPTIONS") || (method() == "TRACE"))
    {
        body_length_ = 0;
        body_size_ = 0;
        return true;
    }

    // Check if the body was fully parsed
    if (body_length_provided_ && (body_size_ >= body_length_))
    {
        body_size_ = body_length_;
        return true;
    }

    return false;
}

std::string_view HttpRequest::fast_convert(std::size_t value, char* buffer, std::size_t size)
{
    std::size_t index = size;
    do
    {
        buffer[--index] = '0' + (value % 10);
        value /= 10;
    }
    while (value > 0);
    return std::string_view(buffer + index, size - index);
}

std::ostream& operator<<(std::ostream& os, const HttpRequest& request)
{
    os << "Request method: " << request.method() << std::endl;
    os << "Request URL: " << request.url() << std::endl;
    os << "Request protocol: " << request.protocol() << std::endl;
    os << "Request headers: " << request.headers() << std::endl;
    for (std::size_t i = 0; i < request.headers(); ++i)
    {
        auto header = request.header(i);
        os << std::get<0>(header) << ": " << std::get<1>(header) << std::endl;
    }
    os << "Request body:" << request.body_length() << std::endl;
    os << request.body() << std::endl;
    return os;
}

void HttpRequest::swap(HttpRequest& request) noexcept
{
    using std::swap;
    swap(error_, request.error_);
    swap(method_index_, request.method_index_);
    swap(method_size_, request.method_size_);
    swap(url_index_, request.url_index_);
    swap(url_size_, request.url_size_);
    swap(protocol_index_, request.protocol_index_);
    swap(protocol_size_, request.protocol_size_);
    swap(headers_, request.headers_);
    swap(cookies_, request.cookies_);
    swap(body_index_, request.body_index_);
    swap(body_size_, request.body_size_);
    swap(body_length_, request.body_length_);
    swap(body_length_provided_, request.body_length_provided_);
    swap(cache_, request.cache_);
    swap(cache_size_, request.cache_size_);
}

std::tuple<std::string_view, std::string_view> HttpRequest::header(std::size_t i) const noexcept
{
  assert((i < headers_.size()) && "Index out of bounds!");
  if (i >= headers_.size())
    return std::make_tuple(std::string_view(), std::string_view());

  auto item = headers_[i];

  return std::make_tuple(
        std::string_view(cache_.data() + std::get<0>(item), std::get<1>(item)),
        std::string_view(cache_.data() + std::get<2>(item), std::get<3>(item)));
}

std::tuple<std::string_view, std::string_view> HttpRequest::cookie(std::size_t i) const noexcept
{
  assert((i < cookies_.size()) && "Index out of bounds!");
  if (i >= cookies_.size())
    return std::make_tuple(std::string_view(), std::string_view());

  auto item = cookies_[i];

  return std::make_tuple(
        std::string_view(cache_.data() + std::get<0>(item), std::get<1>(item)),
        std::string_view(cache_.data() + std::get<2>(item), std::get<3>(item)));
}

}}}
