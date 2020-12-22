#include "Server/SslSession.hpp"
#include "Server/SslServer.hpp"


namespace webcrown {
namespace server {

SslSession::SslSession(
    const std::shared_ptr<SslServer>& server)
  : bytes_pending_(0)
  , bytes_sending_(0)
  , bytes_received_(0)
  , bytes_sent_(0)
  , server_(server)
  , io_service_(server_->service()->asio_service())
  , context_(asio::ssl::context::sslv23)
  , stream_socket_(*io_service_, context_)
{
}

void SslSession::connect()
{
  bytes_pending_ = 0;
  bytes_sending_ = 0;
  bytes_received_ = 0;
  bytes_sent_ = 0;

  connected_ = true;
}

void SslSession::try_receive()
{
  auto async_receive_handler = [this](std::error_code ec, std::size_t bytes_size)
  {
    // Received some data from the client
    if (bytes_size > 0)
    {
      bytes_received_ += bytes_size;

    }
  };

  stream_socket_.async_read_some(
        asio::buffer(receive_buffer_.data(), receive_buffer_.size()),
        async_receive_handler);
}

}}
