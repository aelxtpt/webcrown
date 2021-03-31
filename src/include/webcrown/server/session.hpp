#pragma once
#include "webcrown/server/service.hpp"
#include <asio/ssl/stream.hpp>

namespace webcrown {
namespace server {

class server;

/// This class mantains a socket with the SSL Client. Responsable to read and write
/// data with the SSL Client.
class session
{
  // The class SslServer is responsable
  // to connect and disconnect the session
  friend class server;

  // Session Statistic
  uint64_t bytes_pending_;
  uint64_t bytes_sending_;
  uint64_t bytes_received_;
  uint64_t bytes_sent_;

  /// Server & session
  std::shared_ptr<server> server_;

  /// Asio IO service
  std::shared_ptr<asio::io_service> io_service_;

  /// Session Socket
  asio::ssl::stream<asio::ip::tcp::socket> stream_socket_;

  /// Connected flag
  std::atomic<bool> connected_;

  /// Handshake flag
  /// The main purpose of an SSL handshake is to provide
  /// privacy and data integrity for communication between
  /// a server and a client
  std::atomic<bool> handshaked_;

  /// Receiving flag
  std::atomic<bool> receiving_;

  // The session ID
  uint64_t session_id_;

  // Receive Buffer
  std::vector<uint8_t> receive_buffer_;

  // Logger
  std::shared_ptr<spdlog::logger> logger_;
public:
  /// Initialize the session with a given server
  ///
  /// \param session_id - unique identifier of the session
  /// \param server - Connected server
  explicit session(
      uint64_t session_id,
      std::shared_ptr<webcrown::server::server> const& server,
      std::shared_ptr<spdlog::logger> const& logger,
      std::shared_ptr<asio::ssl::context> const& context);

  session(session const&) = delete;
  session(session &&) = delete;

  session& operator=(session const&) = delete;
  session& operator=(session&&) = delete;

  ~session() = default;

  /// Get the connected server
  std::shared_ptr<server>& server() noexcept { return server_; }

  /// Get the number of bytes pending sent by the session
  uint64_t bytes_pending() const noexcept { return bytes_pending_; }

  /// Get the number of bytes sent by the session
  uint64_t bytes_sent() const noexcept { return bytes_sending_; }

  /// Get the number of bytes received by the session
  uint64_t bytes_received() const noexcept { return bytes_received_; }

  /// Return true if the session is connected
  bool is_connected() const noexcept { return connected_; }

  /// Return true if the session is handshaked
  bool is_handshaked() const noexcept { return handshaked_; }

  /// Get the session unique identifier
  uint64_t session_id() const noexcept { return session_id_; }

  /// Return the size of the receive buffer
  std::size_t option_receive_buffer_size() const;

  asio::ssl::stream<asio::ip::tcp::socket>::next_layer_type& socket() noexcept
  { return stream_socket_.next_layer(); }


  /// Disconnect the session
  ///
  /// \param error - error code reason
  /// \return 'true' if the session was successfully disconnected, 'false' if the session is already disconnected
  bool disconnect(std::error_code error = {});
private:

  /// Disconnect the session async
  ///
  /// \param dispatch - if true, the dispatch io_service is used instead post
  /// \return 'true' if the session was successfully disconnected, 'false' if the sesson is already disconnected
  bool disconnect_async(bool dispatch);

  /// Connect the session
  void connect();

  /// Try to receive new data
  void try_receive();

  /// Event dispatched when the session is connected
  virtual void on_connected() {};

  /// Event dispatched when the session is handshaked
  virtual void on_handshaked() {};

  /// Event dispatched when the session is disconnected
  ///
  /// \param error - error code reason
  virtual void on_disconnected(std::error_code error) {}

  /// Event dispatched when the session receive any bytes 
  ///
  /// \param buffer - received buffer
  /// \param size - size of the received buffer
  virtual void on_received(void const* buffer, std::size_t size) {}
};

}}
