#include "Server/Http/HttpSession.hpp"
#include "Server/Http/HttpServer.hpp"
#include "Server/Http/HttpContext.hpp"
#include "Server/Http/HttpRequest.hpp"

#include <algorithm>

namespace webcrown {
namespace server {
namespace http {

HttpSession::HttpSession(
    uint64_t session_id,
    std::shared_ptr<HttpServer> server,
    std::shared_ptr<spdlog::logger> const& logger,
    std::shared_ptr<asio::ssl::context> const& context)
  : Session(session_id, server, logger, context)
{
}

void HttpSession::on_received(void const* buffer, std::size_t size)
{
  std::shared_ptr<HttpEndpoint> endpoint_handler{nullptr};

  // Receive HTTP Request header
  if (request_.is_pending_header())
  {
    if (request_.receive_header(buffer, size))
    {
      // onReceiveRequestHeader

      // search url in controllers,
      // and invoke endpoint
      std::string url = std::string(request_.url());

      auto context = std::make_shared<HttpContext>(shared_from_this());
      for(auto const& c : controllers_)
      {
        c->on_setup(context);
      }

      auto controller = std::find_if(controllers_.begin(), controllers_.end(),
                   [url, &endpoint_handler](std::shared_ptr<IController> const controller)
      {
        auto&& endpoints = controller->endpoints();

        auto endpoint = std::find_if(endpoints.begin(), endpoints.end(),
                                     [url, &endpoint_handler](std::shared_ptr<HttpEndpoint> const& endp)
        {
            return url == endp->uri();
        });

        if (endpoint != endpoints.end())
        {
          endpoint_handler = *endpoint;
          return true;
        }

        return false;
      });
    }

    size = 0;
  }

  // Check for HTTP Request error
  if (request_.error())
  {
    // onReceivedRequestError(_request, "Invalid HTTP request!");
    request_.clear();
    disconnect();
    return;
  }

  // Receive HTTP request body
  if (!request_.receive_body(buffer, size))
  {
    request_.clear();
    return;
  }

  if(request_.error())
  {
    request_.clear();
    disconnect();
    return;
  }

  if(endpoint_handler != nullptr)
  {
    //  found
    auto&& cb = endpoint_handler->callback();
    cb(request_);
  }
}

}}}
