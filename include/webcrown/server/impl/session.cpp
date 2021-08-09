#include "webcrown/server/session.hpp"
#include "webcrown/server/server.hpp"

namespace webcrown {
namespace server {

session::session(
    uint64_t session_id,
    std::shared_ptr<webcrown::server::server> const& server,
    std::shared_ptr<spdlog::logger> const& logger)
    : bytes_pending_(0)
    , bytes_sending_(0)
    , bytes_received_(0)
    , bytes_sent_(0)
    , server_(server)
    , io_service_(server_->service1()->asio_service())
    , socket_(*io_service_)
    , connected_(false)
//    , handshaked_(false)
    , receiving_(false)
    , session_id_(session_id)
    , logger_(logger)
    , send_buffer_flush_offset(0)
    , sending_(false)
{}

void session::connect()
{
    bytes_pending_ = 0;
    bytes_sending_ = 0;
    bytes_received_ = 0;
    bytes_sent_ = 0;

    receive_buffer_.resize(option_receive_buffer_size());

    // Update the connected flag
    connected_ = true;

    // Call event
    on_connected();

    // The SSL need handshake
    // But for simple TCP, we only need read
    try_receive();
    // handshake
//    stream_socket_.async_handshake(
//        asio::ssl::stream_base::server,
//        std::bind(this, &session<SocketSecurityT>::handshake_secure_handler));
}

bool session::disconnect(std::error_code error)
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
    //handshaked_ = false;
    connected_ = false;

    on_disconnected(error);

    auto unregister_session_handler = [this]()
    {
        server_->unregister_session(session_id_);
    };

    server_->service1()->dispatch(unregister_session_handler);

    return true;
}

bool session::disconnect_async(bool dispatch)
{
    if (!is_connected())
    {
        logger_->error("[Session][disconnect_async] Session is not connected");
        return false;
    }

    auto disconnect_handler = [this]() -> void
    {
        if (!is_connected())
        {
            logger_->error("[Session][disconnect_async][disconnect_handler] Session is not connected");
            return;
        }

        // Cancel the socket socket
        std::error_code ec;
        socket().close(ec);

        if (ec)
        {
            logger_->error(
                "[Session][disconnect_async][disconnect_handler] Error on close socket. Code: {}. Message: {}",
                ec.value(),
                ec.message());
            return;
        }

        auto async_shutdown_handler = [this](std::error_code ec) { disconnect(ec); };

        // Async shutdown with the shutdown handler
        //stream_socket_.async_shutdown(async_shutdown_handler);
        disconnect(ec);
    };

    if (dispatch)
        io_service_->dispatch(disconnect_handler);
    else
        io_service_->post(disconnect_handler);

    return true;
}

void session::try_receive()
{
    if (receiving_)
    {
        logger_->warn("[SslSession][try_receive] Session is already in process of receiving...");
        return;
    }

//    if (!is_handshaked())
//    {
//        logger_->warn("[SslSession][try_receive] Session is not handshaked, but you should not to worry, because this is probably the second read");
//        return;
//    }

    receiving_ = true;

    auto async_receive_handler = [this](asio::error_code const& ec, std::size_t bytes_size)
    {
        receiving_ = false;

        logger_->info("[Session][try_receive][async_receive_handler] receiving message {} bytes",
                      bytes_size);

//        if (!is_handshaked())
//        {
//            logger_->error("[SslSession][try_receive][async_receive_handler] Session is not handshaked");
//            return;
//        }

        // Received some data from the client
        if (bytes_size > 0)
        {
            // Update statistic
            bytes_received_ += bytes_size;
            //server()->add_bytes_received(bytes_size); // TODO: race condition ?

            // Dispatch event
            on_received(receive_buffer_.data(), bytes_size);

            // if the receive buffer is full, so increase its size
            if (receive_buffer_.size() == bytes_size)
                receive_buffer_.resize(2 * bytes_size);
        }

        // Try to receive again if the session is valid
        if (!ec)
            try_receive();
        else
            disconnect(ec);
    };

    socket_.async_read_some(
        asio::buffer(receive_buffer_.data(), receive_buffer_.size()),
        async_receive_handler);
}

std::size_t session::option_receive_buffer_size() const
{
    asio::socket_base::receive_buffer_size option;
    socket_.get_option(option);
    return option.value();
}

size_t session::send(void const* buffer, size_t size)
{
//    if (!is_handshaked())
//    {
//        logger_->error("[SslSession][send] Session is not handshaked");
//        return 0;
//    }

    if (size == 0)
    {
        logger_->error("[SslSession][send] size is zero");
        return 0;
    }

    assert((buffer != nullptr) && "Pointer to the buffer should not be null!");
    if (buffer == nullptr)
    {
        logger_->error("[SslSession][send] pointer to the buffer is null");
        return 0;
    }

    asio::error_code ec;

    // Send data to the client
    size_t sent = asio::write(socket_, asio::buffer(buffer, size), ec);
    if (sent <= 0)
    {
        send_error(ec);
        disconnect(ec);
        return sent;
    }

    // Update statistic
    bytes_sent_ += sent;
    // server_->add_bytes_sent(sent); // TODO: Race condition ?

    // call the buffer sent handler
    on_sent(sent, bytes_pending());

    return sent;
}

bool
session::send_async(void const* buffer, size_t size)
{
//    if(!is_handshaked())
//    {
//        logger_->error("[SslSession][send_async] Session is not handshaked");
//        return 0;
//    }

    if (size == 0)
    {
        logger_->error("[SslSession][send_async] size is zero");
        return 0;
    }

    assert((buffer != nullptr) && "Pointer to the buffer should not be null!");
    if (buffer == nullptr)
    {
        logger_->error("[SslSession][send_async] pointer to the buffer is null");
        return 0;
    }

    {
        std::scoped_lock locker(send_lock_);

        // Detect multiple send handlers
        bool send_required = send_buffer_main_.empty() || send_buffer_flush_.empty();

        // Fill the main send buffer
        uint8_t const* bytes = (uint8_t const*)buffer;
        send_buffer_main_.insert(send_buffer_main_.end(), bytes, bytes + size);

        // Update Statistics
        bytes_pending_ = send_buffer_main_.size();

        // Avoid multiple send handlers
        if (!send_required)
            return true;
    }

    // Dispatch the send handler
    auto send_handler = [this]()
    {
        try_send();
    };

    io_service_->dispatch(send_handler);

    return true;
}

void
session::try_send()
{
    if (sending_)
    {
        logger_->error("[Session][send_try_send] It already sending...");
        return;
    }

//    if (!is_handshaked())
//    {
//        logger_->error("[SslSession][try_send] Session is not handshaked");
//        return;
//    }

    // Swap send buffers
    if (send_buffer_flush_.empty())
    {
        std::scoped_lock locker(send_lock_);

        // Swap flush and main buffers
        send_buffer_flush_.swap(send_buffer_main_);
        send_buffer_flush_offset = 0;

        // Update statistic
        bytes_pending_ = 0;
        bytes_sending_ += send_buffer_flush_.size();
    }

    // Check if the flush buffer is empty
    // because buffer_main can be empty
    if (send_buffer_flush_.empty())
    {
        // Call the empty send buffer handler
        on_empty();
        return;
    }

    // Async write with the write handler
    sending_ = true;
    auto async_write_handler = [this](std::error_code ec, size_t size)
    {
        sending_ = false;

//        if (!is_handshaked())
//        {
//            logger_->error("[SslSession][try_send_async_write_handler] is not handshaked!");
//                return;
//        }

        // Send some data to the client
        if (size <= 0)
        {
	        logger_->error("[SslSession][try_send_async_write_handler] size is zero");
            return;
        }

        // Update statistics
        bytes_sending_ -= size;
        bytes_sent_ += size;
        // server_->add_bytes_sent(size); TODO: race condition ?

        // Increase the flush buffer offset
        send_buffer_flush_offset += size;

        // Successfully send the whole flush buffer
        if (send_buffer_flush_offset == send_buffer_flush_.size())
        {
            // Clear the flush buffer
            send_buffer_flush_.clear();
            send_buffer_flush_offset = 0;
        }

        // Call the buffer sent handler
        on_sent(size, bytes_pending());

        // Try to send again if the session is valid
        if (!ec)
            try_send();
        else
        {
            send_error(ec);
            disconnect(ec);
        }
    };

    socket_.async_write_some(
        asio::buffer(send_buffer_flush_.data() + send_buffer_flush_offset,
                     send_buffer_flush_.size() - send_buffer_flush_offset),
        async_write_handler);
}

void
session::send_error(std::error_code ec)
{
    // Skip asio disconnect errors
    if ((ec == asio::error::connection_aborted) ||
        (ec == asio::error::connection_refused) ||
        (ec == asio::error::connection_reset) ||
        (ec == asio::error::eof) ||
        (ec == asio::error::operation_aborted))
        return;

    // Skip OpenSSL annoying errors
    if (ec == asio::ssl::error::stream_truncated)
        return;
    if (ec.category() == asio::error::get_ssl_category())
    {
        if ((ERR_GET_REASON(ec.value()) == SSL_R_DECRYPTION_FAILED_OR_BAD_RECORD_MAC) ||
            (ERR_GET_REASON(ec.value()) == SSL_R_PROTOCOL_IS_SHUTDOWN) ||
            (ERR_GET_REASON(ec.value()) == SSL_R_WRONG_VERSION_NUMBER))
            return;
    }

    on_error(ec.value(), ec.category().name(), ec.message());
}
//
//template<typename SocketSecurityT>
//template<SocketSecurityT>
//typename std::enable_if<(std::is_same<SocketSecurityT, SecureSocket>::value)>::type
//session<SocketSecurityT>::handshake_secure_handler(asio::error_code ec)
//{
//    if (is_handshaked())
//    {
//        logger_->warn("[SslSession][connect][async_handshake_handler] The session is already handshaked");
//        return;
//    }
//
//    if (ec)
//    {
//        logger_->error(
//        "[SslSession][connect][async_handshake_handler] Error on handshake process. Code: {}. Message: {}",
//        ec.value(),
//        ec.message());
//
//        // disconnect in case of the bad handshake
//        disconnect(ec);
//        return;
//    }
//
//    // Update the handshaked flag
//    handshaked_ = true;
//
//    // Call event
//    on_handshaked();
//
//    // Try to receive something from the client
//    try_receive();
//}

}
}
