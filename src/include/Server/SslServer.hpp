#pragma once
#include "Server/SslSession.hpp"
#include <memory>

namespace webcrown {
namespace server {

/// This class
class SslServer : std::enable_shared_from_this<SslServer>
{
  bool started_;

  asio::ip::tcp::acceptor socket_acceptor_;
  std::shared_ptr<asio::io_service> io_service_;


  std::shared_ptr<spdlog::logger> logger_;

  // Server session
  std::shared_ptr<SslSession> session_;

  // Asio Service
  std::shared_ptr<Service> service_;

  // Server statistics
  uint64_t bytes_pending_;
  uint64_t bytes_sent_;
  uint64_t bytes_received_;

  // Server config
  std::string address_;
  uint16_t port_number_;
public:

  /// Initialize SSL server with a given Asio service, SSL Context and port number and address
  ///
  /// \param service - Asio service
  /// \param port_num - Port number
  /// \param address - An ipv4 or ipv6 address
  explicit SslServer(
    std::shared_ptr<spdlog::logger> logger,
    std::shared_ptr<Service> const& service,
    uint16_t port_num,
    std::string_view address);

  /// Start the server
  bool start();

  bool is_started() const noexcept { return started_; }

  // Get the Asio Service
  std::shared_ptr<Service>& service() noexcept { return service_; }

private:
  void accept();
};

}}
