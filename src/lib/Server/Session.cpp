#include "Server/Session.hpp"
#include "Server/Server.hpp"


namespace webcrown {
namespace server {

Session::Session(
    uint64_t session_id,
    std::shared_ptr<Server> const& server,
    std::shared_ptr<spdlog::logger> const& logger)
  : bytes_pending_(0)
  , bytes_sending_(0)
  , bytes_received_(0)
  , bytes_sent_(0)
  , server_(server)
  , io_service_(server_->service()->asio_service())
  , context_(asio::ssl::context::sslv23)
  , stream_socket_(*io_service_, context_)
  , connected_(false)
  , handshaked_(false)
  , receiving_(false)
  , session_id_(session_id)
  , logger_(logger)
{
}

void Session::connect()
{
  bytes_pending_ = 0;
  bytes_sending_ = 0;
  bytes_received_ = 0;
  bytes_sent_ = 0;

  // Update the connected flag
  connected_ = true;

  // Call event
  on_connected();

  auto async_handshake_handler = [this](std::error_code ec)
  {
    if (is_handshaked())
    {
      logger_->warn("[SslSession][connect][async_handshake_handler] The session is already handshaked");
      return;
    }

    if (ec)
    {
      logger_->error("[SslSession][connect][async_handshake_handler] Error on handshake process. Code: {}. Message: {}",
                     ec.value(),
                     ec.message());

      // disconnect in case of the bad handshake
      disconnect(ec);
      return;
    }

    // Update the handshaked flag
    handshaked_ = true;

    // Call event
    on_handshaked();

    // Try to receive something from the client
    try_receive();
  };

  // handshake
  stream_socket_.async_handshake(asio::ssl::stream_base::server, async_handshake_handler);
}

bool Session::disconnect(std::error_code error)
{
  if (!is_connected())
  {
    logger_->error("[SslSession][disconnect] The server is not started");
    return false;
  }

  if (error)
  {
    logger_->error("[SslSession][disconnect] The server will be disconnect with reason: {}",
		   error.message());
  }

  // Close socket
  asio::error_code ec;
  socket().close(ec);

  if (ec.value())
  {
    logger_->error("[SslSession][disconnect] Error on close socket. Code: {}. Message: {}",
		   ec.value(),
		   ec.message());

    return false;
  }

  // Update flags
  handshaked_ = false;
  connected_ = false;

  on_disconnected(error);

  auto unregister_session_handler = [this]()
  {
    server_->unregister_session(session_id_);
  };

  server_->service()->dispatch(unregister_session_handler);

  return true;
}

bool Session::disconnect_async(bool dispatch)
{
  if (!is_connected())
  {
    logger_->error("[SslSession][disconnect_async] Session is not connected");
    return false;
  }

  auto disconnect_handler = [this]() -> void
  {
    if (!is_connected())
    {
      logger_->error("[SslSession][disconnect_async][disconnect_handler] Session is not connected");
      return;
    }

    // Cancel the socket socket
    std::error_code ec;
    socket().close(ec);

    if(ec)
    {
      logger_->error("[SslSession][disconnect_async][disconnect_handler] Error on close socket. Code: {}. Message: {}",
		   ec.value(),
		   ec.message());
      return;
    }

    auto async_shutdown_handler = [this](std::error_code ec) { disconnect(ec); };

    // Async shutdown with the shutdown handler
    stream_socket_.async_shutdown(async_shutdown_handler);
  };

  if (dispatch)
    io_service_->dispatch(disconnect_handler);
  else
    io_service_->post(disconnect_handler);

  return true;
}

void Session::try_receive()
{
  if (receiving_)
  {
    logger_->warn("[SslSession][try_receive] Session is already in process of receiving...");
    return;
  }

  if (!is_handshaked())
  {
    logger_->error("[SslSession][try_receive] Session is not handshaked");
    return;
  }

  receiving_ = true;

  auto async_receive_handler = [this](std::error_code ec, std::size_t bytes_size)
  {
    receiving_ = false;

    if (!is_handshaked())
    {
      logger_->error("[SslSession][try_receive][async_receive_handler] Session is not handshaked");
      return;
    }

    // Received some data from the client
    if (bytes_size > 0)
    {
      // Update statistic
      bytes_received_ += bytes_size;
      server()->add_bytes_received(bytes_size);

      // Dispatch event
      on_received(receive_buffer_.data(), bytes_size);

      // if the receive buffer is full, so increase its size
      // TODO: nao entendi
      if (receive_buffer_.size() == bytes_size)
        receive_buffer_.resize(2 * bytes_size);
    }

    // Try to receive again if the session is valid
    if (ec)
      try_receive();
    else
      disconnect(ec);
  };

  stream_socket_.async_read_some(
        asio::buffer(receive_buffer_.data(), receive_buffer_.size()),
        async_receive_handler);
}

}}
