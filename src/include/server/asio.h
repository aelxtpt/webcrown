#ifndef SERVER_ASIO_H
#define SERVER_ASIO_H

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <thread>
#include <atomic>
#include <iostream>
#include <map>
#include <string>
#include <memory>

namespace webcrown {
namespace server {

enum class InternetProtocol
{
  IPv4,
  IPv6
};


class Service {
  static const std::map<unsigned int, std::string> http_status_table;
public:
  Service() {}
  explicit Service(std::shared_ptr<asio::ip::tcp::socket> sock)
    : m_sock(sock)
    , m_request(4096)
    , m_response_status_code(200) // Assume  success.
    , m_resource_size_bytes(0)
  {
  }

  void start_handling() {
    asio::async_read_until(*m_sock.get(),
                           m_request,
                           "\r\n",
                           [this](asio::error_code const& ec,
                                  std::size_t bytes_transferred)
    {
      on_request_line_received(ec, bytes_transferred);
    });
  }
private:
  void on_request_line_received(asio::error_code const& ec, std::size_t bytes_transferred)
  {
    if(ec.value() != 0) {
      std::cout << "Error occured! Error code = "
        << ec.value()
        << ". Message: " << ec.message();

      if(ec == asio::error::not_found) {
        // No delimiterhas been found in the request message
        m_response_status_code = 413;
        send_response();

        return;
      }
      else {
        //In case of any other error
        // close the socket and clean up.
        on_finish();
        return;
      }
    }

    // Parse the request line.
    std::string request_line;
    std::istream request_stream(&m_request);
    std::getline(request_stream, request_line, '\r');

    // Remove the symbol '\n' from the buffer.
    request_stream.get();

    // Parse the request line.
    std::string request_method;
    std::istringstream request_line_stream(request_line);
    request_line_stream >> request_method;

    // We only support GET METHOD
    if (request_method.compare("GET") !=0) {
      //Unsupported method.
      m_response_status_code = 501;
      send_response();

      return;
    }

    request_line_stream >> m_requested_resource;

    std::string request_http_version;
    request_line_stream >> request_http_version;

    if(request_http_version.compare("HTTP/1.1") != 0) {
      //Unsupported http version or bad request.
      m_response_status_code = 505;
      send_response();

      return;
    }

    // At this pointthe request line is successfully
    // received and parsed. Now read the request headers.
    asio::async_read_until(*m_sock.get(),
                           m_request,
                           "\r\n\r\n",
                           [this](asio::error_code const& ec,
                                  std::size_t bytes_transferred)
    {
      on_headers_received(ec, bytes_transferred);
    });

    return;
  }

  void on_headers_received(asio::error_code const& ec, std::size_t bytes_transferred)
  {
    if(ec.value() != 0) {
      std::cout << "Error occured! Error code = "
      << ec.value()
      << ". Message: " << ec.message();

      if (ec == asio::error::not_found) {
        // No delimiter has been found in the
        // request message

        m_response_status_code = 413;
        send_response();
        return;
      }
      else {
        // In case of any other error - close the
        // socket and clean up
        on_finish();
        return;
      }
    }

    // Parse and store headers.
    std::istream request_stream(&m_request);
    std::string header_name, header_value;

    while (!request_stream.eof()) {
      std::getline(request_stream, header_name, ':');
      if(!request_stream.eof()) {
        std::getline(request_stream, header_value, '\r');
      }

      // Remove symbol \n from the stream.
      request_stream.get();
      m_request_headers[header_name] = header_value;
    }

    // now we have all we need to process the request.
    process_request();
    send_response();
  }

  void process_request() {
    // Read file.
  }

  void send_response() {
    m_sock->shutdown(asio::ip::tcp::socket::shutdown_receive);

    auto status_line = 
      http_status_table.at(m_response_status_code);

    m_response_status_line = std::string("HTTP/1.1 ") +
      status_line +
      "\r\n";

    m_response_headers += "\r\n";

    std::vector<asio::const_buffer> response_buffers;
    response_buffers.push_back(asio::buffer(m_response_status_line));

    if (m_response_headers.length() > 0) {
      response_buffers.push_back(asio::buffer(m_response_headers));
    }

    if (m_resource_size_bytes > 0) {
      response_buffers.push_back(
        asio::buffer(m_resource_buffer.get(),
                     m_resource_size_bytes));
    }

    // Initiate asynchronous write operation.
    asio::async_write(*m_sock.get(),
                      response_buffers,
                      [this](asio::error_code const& ec,
                             std::size_t bytes_transferred)
    {
      on_response_sent(ec, bytes_transferred);
    });
  }

  void on_response_sent(asio::error_code const& ec, std::size_t bytes_transferred) {
    if (ec.value() != 0) {
      std::cout << "Error occured! Error code = "
        << ec.value()
        << ". Message: " << ec.message();
    }

    m_sock->shutdown(asio::ip::tcp::socket::shutdown_both);

    on_finish();
  }

  void on_finish() {
    delete this;
  }


private:
  std::shared_ptr<asio::ip::tcp::socket> m_sock;
  asio::streambuf m_request;
  std::map<std::string, std::string> m_request_headers;
  std::string m_requested_resource;

  std::unique_ptr<char[]> m_resource_buffer;
  unsigned int m_response_status_code;
  std::size_t m_resource_size_bytes;
  std::string m_response_headers;
  std::string m_response_status_line;
};

const std::map<unsigned int, std::string>
Service::http_status_table = 
{
  { 200, "200 OK" },
  { 404, "404 Not Found" },
  { 413, "413 Request Entity Too Large" },
  { 500, "500 Server Error" },
  { 501, "501 Not Implement" },
  { 505, "505 Http Version Not Supported" }
};

}}

#endif
