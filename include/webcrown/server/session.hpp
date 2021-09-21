#pragma once

#include "webcrown/server/service.hpp"

namespace webcrown {
namespace server {

class server;

/// This class mantains a socket with the Client. Responsable to read and write
/// data with the Client.
class session
{
    // The class Server is responsable
    // to connect and disconnect the session
    friend class server;

    // Session Statistic
    uint64_t bytes_pending_;
    uint64_t bytes_sending_;
    uint64_t bytes_received_;
    uint64_t bytes_sent_;

    /// Server & session
    std::shared_ptr <webcrown::server::server> server_;

    /// Asio IO service
    std::shared_ptr <asio::io_service> io_service_;

    /// Session Socket
    asio::ip::tcp::socket socket_;

    /// Connected flag
    std::atomic<bool> connected_;

    /// Receiving flag
    std::atomic<bool> receiving_;

    // The session ID
    uint64_t session_id_;

    // Receive Buffer
    std::vector<uint8_t> receive_buffer_;

    // Logger
    std::shared_ptr <spdlog::logger> logger_;

    // Lock for the send
    std::mutex send_lock_;

    std::vector<uint8_t> send_buffer_main_;
    std::vector<uint8_t> send_buffer_flush_;
    size_t send_buffer_flush_offset;

    // Flag for sending buffer
    bool sending_;
public:
    /// Initialize the session with a given server
    ///
    /// \param session_id - unique identifier of the session
    /// \param server - Connected server
    explicit session(
        uint64_t session_id,
        std::shared_ptr <webcrown::server::server> const& server,
        std::shared_ptr <spdlog::logger> const& logger);

    session(session const&) = delete;
    session(session&&) = delete;

    session& operator=(session const&) = delete;
    session& operator=(session&&) = delete;

    virtual ~session() = default;

    /// Get the connected server
    std::shared_ptr <webcrown::server::server>& server()  noexcept
    { return server_; }

    /// Get the number of bytes pending sent by the session
    uint64_t bytes_pending() const noexcept
    { return bytes_pending_; }

    /// Get the number of bytes sent by the session
    uint64_t bytes_sent() const noexcept
    { return bytes_sending_; }

    /// Get the number of bytes received by the session
    uint64_t bytes_received() const noexcept
    { return bytes_received_; }

    /// Return true if the session is connected
    bool is_connected() const noexcept
    { return connected_; }

    /// Get the session unique identifier
    uint64_t session_id() const
    noexcept { return session_id_; }

    /// Return the size of the receive buffer
    std::size_t option_receive_buffer_size() const;

    asio::ip::tcp::socket& socket() noexcept
    { return socket_; }


    /// Disconnect the session
    ///
    /// \param error - error code reason
    /// \return 'true' if the session was successfully disconnected, 'false' if the session is already disconnected
    bool disconnect(std::error_code error = {});

    /// Connect the session
    void connect();

    virtual size_t send(void const* buffer, size_t size);

    virtual size_t send(std::string_view text) { return send(text.data(), text.size()); }

    virtual bool send_async(void const* buffer, size_t size);

    virtual bool send_async(std::string_view text) { return send_async(text.data(), text.size()); }
    
private:
    void clear_buffers();
    
    /// Try to receive new data
    virtual void try_receive();

    /// Try to send pending data
    void try_send();

//    template<SocketSecurityT>
//    typename std::enable_if<(std::is_same<SocketSecurityT, SecureSocket>::value)>::type
//    handshake_secure_handler(asio::error_code ec);

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

    /// Handle buffer sent notification
    /// Notification is called when another chunk of buffer was sent
    /// to the client.
    ///
    /// This handler could be used to send another buffer to the client
    /// for instance when the pending size is zero.
    ///
    /// \param sent - Size of sent buffer
    /// \param pending - Size of pending buffer
    virtual void on_sent(size_t sent, size_t pending) {}

    /// Handle empty send buffer notification
    /// Notification is called when the send buffer is empty and ready
    /// for a new data to send.
    ///
    /// This handler could be used to send another buffer to the client
    virtual void on_empty() {}

    /// Handle error notification
    /// \param error - Error code
    /// \param category - Error category
    /// \param message - Error message
    virtual void on_error(int error, std::string const& category, std::string const& message) {}

    /// Send error notification
    void send_error(std::error_code ec);


};

}
}
