#include "Server/SslServer.hpp"
#include <assert.h>


namespace webcrown {
namespace server {

SslServer::SslServer(
  std::shared_ptr<spdlog::logger> logger,
  std::shared_ptr<Service> const& service,
  uint16_t port_num,
  std::string_view address)
  : started_(false)
  , socket_acceptor_(*service->asio_service())
  , io_service_(service->asio_service())
  , logger_(logger)
  , service_(service)
  , bytes_pending_(0)
  , bytes_sent_(0)
  , bytes_received_(0)
  , address_(address)
  , port_number_(port_num)
{
}

bool SslServer::start()
{
  assert(!is_started() && "SSL Server is already started");
  if (is_started())
  {
    logger_->warn("[SslServer][start] SSL Server is already started");
    return false;
  }

  asio::ip::address ip_address = asio::ip::address::from_string("127.0.0.1");
  asio::ip::tcp::endpoint endpoint(ip_address, port_number_);

  auto endpoint_family = endpoint.protocol().family();
  auto endpoint_protocol = endpoint.protocol().protocol();

  logger_->info(R"([SslServer][start]
The endpoint ip_address is {}
port number is {}
protocol family is {}
protocol type is {}
protocol {})",
                ip_address.to_string(),
                port_number_,
                endpoint_family == 2 ? "IP protocol family" : std::to_string(endpoint_family),
                endpoint.protocol().type(),
                endpoint_protocol == 6 ? "Transmission Control Protocol" : std::to_string(endpoint_protocol));

  // Post the start handler
  auto start_handler = [this, &endpoint]()
  {
    if (is_started())
    {
      logger_->warn("[SslServer][start][start_handler] SSL Server is already started");
      return;
    }

    asio::error_code ec;

    // Open socket
    socket_acceptor_.open(endpoint.protocol(), ec);

    if (ec.value())
    {
      logger_->error("[SslServer][start] Error on open acceptor socket. Code: {}. Message: {}",
                     ec.value(),
                     ec.message());

      assert(ec.value() == 0 && "Error on open acceptor socket.");
      return;
    }

    // Bind endpoint
    socket_acceptor_.bind(endpoint, ec);
    if (ec.value())
    {
      logger_->error("[SslServer][start] Error on bind acceptor socket. Code: {}. Message: {}",
                     ec.value(),
                     ec.message());

      assert(ec.value() == 0 && "Error on bind acceptor socket.");
      return;
    }

    // Notify the OS that we want to listening for incoming connection requests
    // on specific endpoint by this call.
    socket_acceptor_.listen();

    // Reset statistics
    bytes_pending_ = 0;
    bytes_sent_ = 0;
    bytes_received_ = 0;

    started_ = true;

    // Perform the first server accept
    accept();
  };

  io_service_->post(start_handler);

  return true;
}

void SslServer::accept()
{
  assert(!is_started() && "SSL Server is not started");
  if (!is_started())
  {
    logger_->warn("[SslServer][accept] SSL Server is not started");
    return;
  }

  // Dispatch the accept handler
  auto self = shared_from_this();
  auto accept_handler = [this, self]()
  {
    if (!is_started())
    {
      logger_->warn("[SslServer][accept][accept_handler] SSL Server is not started");
      return;
    }

    session_ = std::make_shared<SslSession>(self);

    auto async_accept_handler = [this, self](std::error_code ec)
    {

        // Connect a new session

    };

    socket_acceptor_.async_accept(session_->socket(), async_accept_handler);
  };


}

}}
