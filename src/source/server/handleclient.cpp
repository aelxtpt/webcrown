#include "asio/error.hpp"
#include "asio/error_code.hpp"
#include "asio/read.hpp"
#include "asio/read_until.hpp"
#include "server/asio.h"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace webcrown {
namespace server {

const std::map<unsigned int, std::string> 
HandleClientRequest::HttpStatusTable = 
{
  { 200, "200 OK" },
  { 404, "404 Not Found"},
  { 413, "413 Request Entity Too Large" },
  { 500, "500 Server Error" },
  { 501, "501 Not Implemented" },
  { 505, "505 HTTP Version Not Supported" }
};


void HandleClientRequest::startHandling() {
  std::cout
    << "Start handling...\n";

  asio::async_read_until(
    *ActiveSocket.get(),
    RequestBuf,
    "\r\n",
    [this](asio::error_code const& Ec, std::size_t BytesTransferred)
    {
      onRequestReceived(Ec, BytesTransferred);
    });
}

void HandleClientRequest::onRequestReceived(asio::error_code const& Ec,
					    std::size_t BytesTransferred) {
  if(Ec.value() != 0) {
    // TODO: Logger
    std::cout
      << "Error on async_read_until. "
      << "Message: "
      << Ec.message()
      << "\n";
    assert(Ec.value() != 0 && "Error on async_read_until.");
    assert(Ec.value() == asio::error::not_found && "No delimiter has been found. Return http 413");

    onFinish();
    return;
  }

  // Process the request
  // Parse the request line
  std::string RequestLine;
  std::istream RequestStream(&RequestBuf);

  std::getline(RequestStream, RequestLine, '\r');
  // Remove the symbol '\n'  from the buffer.
  RequestStream.get();

  std::string RequestMethod;
  std::istringstream RequestLineStream(RequestLine);
  RequestLineStream >> RequestMethod;

  std::cout
    << "RequestLine ==> "
    << RequestLine
    << "\n";

  // Add support to GET Method
  if(RequestMethod.compare("GET") != 0) {
    // Unsupported method
    ResponseStatusCode = 501;
    // TODO: sendresponse
    //
    return;
  }

  RequestLineStream >> RequestedResource;

  std::string RequestHttpVersion;
  RequestLineStream >> RequestHttpVersion;

  if(RequestHttpVersion.compare("HTTP/1.1") != 0) {
    std::cout
      << "Unsupported http version. "
      << "Message: "
      << Ec.message()
      << "\n";

    // Unsupported HTTP versionor bad request.
    ResponseStatusCode = 505;
    sendResponse();
    
    return;
  }

  // At this point the request line is successfully
  // received and parse. Now read the request headers.
  asio::async_read_until(
    *ActiveSocket.get(),
    RequestBuf,
    "\r\n\r\n",
    [this](asio::error_code const& Ec, std::size_t BytesTransferred)
    {
      onHeadersReceived(Ec, BytesTransferred);
    });
}

void HandleClientRequest::onHeadersReceived(asio::error_code const& Ec,
					    std::size_t BytesTransferred) {
  if(Ec.value() != 0) {
    // TODO: Logger
      std::cout
	<< "onHeadersReceived:: Error on async_read_until. "
	<< "Message: "
	<< Ec.message()
	<< "\n";

    assert(Ec.value() != 0 && "Error on async_read_until headers");
    assert(Ec.value() == asio::error::not_found && "No delimiter has been found. Return http 413");

    onFinish();
    return;
  }

  // Parse and store headers.
  std::istream RequestStream(&RequestBuf);
  std::string HeaderName, HeaderValue;

  while (!RequestStream.eof()) {
    std::getline(RequestStream, HeaderName, ':');
    if (!RequestStream.eof()) {
      std::getline(RequestStream, HeaderValue, '\r');

      // Remove the symbol '\n' from the stream
      RequestStream.get();

      RequestHeaders[HeaderName] = HeaderValue;
    }
  }

  // Now we have all we needto process the request
  processRequest();
  sendResponse();
}

// TODO: change in future
// This read a html file
void HandleClientRequest::processRequest() {
  auto CurPath = std::filesystem::current_path();

  std::cout << "Current path: " << CurPath.string() + "\n";
  std::string ResourceFilepath = CurPath.string() + "/index.html";

  std::ifstream ResourceFstream(ResourceFilepath, std::ios_base::binary);
  if(!ResourceFstream.is_open()) {
    std::cout
      << "Failed to open file "
      << ResourceFilepath
      << "\n";
    // Could no open the file
    // somethinb bad has happened.
    ResponseStatusCode = 500;
    return;
  }

  // Find out
  // file size.
  ResourceFstream.seekg(0, std::ifstream::end);
  ResourceSizeBytes = static_cast<std::size_t>(ResourceFstream.tellg());

  ResourceBuffer.reset(new char[ResourceSizeBytes]);

  ResourceFstream.seekg(std::ifstream::beg);
  ResourceFstream.read(ResourceBuffer.get(), ResourceSizeBytes);

  ResponseHeaders += std::string("content-length") +
    ": "+
    std::to_string(ResourceSizeBytes) +
    "\r\n";
}

void HandleClientRequest::sendResponse() {
  asio::error_code Ec;
  // Disable receive operations
  ActiveSocket->shutdown(asio::ip::tcp::socket::shutdown_receive, Ec);

  if (Ec.value() != 0) {
    // TODO: logger
    std::cout
      << "sendResponse:: Error on shutdown socket. "
      << "Message: "
      << Ec.message()
      << "\n";

    assert(Ec.value() != 0 && "Error on shutdown socket.");
    return;
  }

  auto StatusLine = HttpStatusTable.at(ResponseStatusCode);

  auto ResponseStatusLine = std::string("HTTP/1.1") +
    StatusLine + 
    "\r\n";

  ResponseHeaders += "\r\n";

  std::cout
    << "Response headers\n\n*"
    << ResponseHeaders
    << "*\n\n";

  std::vector<asio::const_buffer> ResponseBuffers;
  ResponseBuffers.push_back(asio::buffer(ResponseStatusLine));

  if (ResponseHeaders.length() > 0) {
    ResponseBuffers.push_back(asio::buffer(ResponseHeaders));
  }

  if (ResourceSizeBytes > 0) {
    ResponseBuffers.push_back(asio::buffer(ResourceBuffer.get(),
					   ResourceSizeBytes));
  }

  // Initiate asynchronous write operation
  asio::async_write(*ActiveSocket.get(),
		    ResponseBuffers,
		    [this](asio::error_code const& Ec, std::size_t BytesTransferred)
  {
    onResponseSent(Ec, BytesTransferred);
  });
}

void HandleClientRequest::onResponseSent(asio::error_code const& Ec,
					 std::size_t BytesTransferred)
{
  if(Ec.value() != 0) {
    // TODO: Logger
    assert(Ec.value() != 0 && "Error on send response to the client.");
    return;
  }

  asio::error_code EcShutdown;

  ActiveSocket->shutdown(asio::ip::tcp::socket::shutdown_both, EcShutdown);

  if(EcShutdown.value() != 0) {
    // TODO: Logger
    assert(Ec.value() != 0 && "Error on shutdown socket.");
  }

  onFinish();
}

void HandleClientRequest::onFinish() {
  // TODO: ponteiro solto ainda?
  delete this;
}

}}
