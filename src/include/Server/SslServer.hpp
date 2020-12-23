#pragma once
#include "Server/SslSession.hpp"
#include <memory>
#include <shared_mutex>
#include <map>

namespace webcrown {
namespace server {

/// This class
class SslServer : public std::enable_shared_from_this<SslServer>
{
  std::atomic<bool> started_;

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

  // Threading sessions
  std::shared_mutex sessions_lock_;
  std::map<uint64_t, std::shared_ptr<SslSession>> sessions_;
  std::atomic<uint64_t> last_generated_session_id_;
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

  /// Register a new session
  void register_session();

  /// Unregister the given session
  ///
  /// \param id - the session id
  void unregister_session(uint64_t id);
};

}}
