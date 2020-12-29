#pragma once
#include "Server/SslSession.hpp"
#include <memory>
#include <shared_mutex>
#include <map>

namespace webcrown {
namespace server {

/// This class is responsible to connect,
/// disconnect and manage SSL Sessions
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

  SslServer(SslServer const&) = delete;
  SslServer(SslServer&&) = delete;

  SslServer& operator=(SslServer const&) = delete;
  SslServer& operator=(SslServer&&) = delete;

  void add_bytes_pending(uint64_t bytes) noexcept { bytes_pending_ += bytes; }
  void add_bytes_sent(uint64_t bytes) noexcept { bytes_sent_ += bytes; }
  void add_bytes_received(uint64_t bytes) noexcept { bytes_received_ += bytes; }

  virtual ~SslServer() = default;

  /// Start the server
  bool start();

  /// Check if the server is started
  bool is_started() const noexcept { return started_; }

  // Get the Asio Service
  std::shared_ptr<Service>& service() noexcept { return service_; }

  /// Unregister the given session
  ///
  /// \param id - the session id
  void unregister_session(uint64_t id);
private:
  void accept();

  /// Create a new session
  ///
  /// \param session_id - unique identifier for the session
  /// \param server - the connected server
  virtual std::shared_ptr<SslSession> create_session(
      uint64_t session_id, std::shared_ptr<SslServer> const& server, std::shared_ptr<spdlog::logger> const& logger);

  /// Register a new session
  ///
  /// A session is responsible to handle
  /// incoming requests and send response to them
  void register_session();
};

}}
