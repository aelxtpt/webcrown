#include "webcrown/server/session.hpp"
#include "webcrown/server/server.hpp"
#include "webcrown/server/error.hpp"

namespace webcrown {
namespace server {

session::session(
    uint64_t session_id,
    std::shared_ptr<webcrown::server::server> const& server)
    : bytes_pending_(0)
    , bytes_sending_(0)
    , bytes_received_(0)
    , bytes_sent_(0)
    , server_(server)
    , io_service_(server_->service1()->asio_service())
    , socket_(*io_service_)
    , connected_(false)
    , receiving_(false)
    , session_id_(session_id)
    , send_buffer_flush_offset(0)
    , sending_(false)
{
    logger_ = server_->logger();
}

void session::connect()
{
    bytes_pending_ = 0;
    bytes_sending_ = 0;
    bytes_received_ = 0;
    bytes_sent_ = 0;

    // test
    //socket_.set_option(asio::ip::tcp::socket::keep_alive(true));

    receive_buffer_.resize(option_receive_buffer_size());

    // Update the connected flag
    connected_ = true;

    // Call event
    on_connected();

    // Start receive data
    try_receive();
}

void
session::clear_buffers()
{
    std::scoped_lock locker(send_lock_);

    // Clear send buffers
    send_buffer_main_.clear();
    send_buffer_flush_.clear();

    // Update statistic
    bytes_pending_ = 0;
    bytes_sending_ = 0;
}

bool session::disconnect(asio::error_code error)
{
    SPDLOG_LOGGER_DEBUG(logger_, "Disconnection session {} with error {}", session_id_, error.message());

    if (!is_connected())
    {
        error = make_error(server_error::server_not_started);
        on_error(error);
        return false;
    }

    // Notify that session will be disconnected with the error
    on_disconnect(error);
    
    auto disconnect_handler = [this]()
    {
        asio::error_code ec;
        if (!is_connected())
        {
            ec = make_error(server_error::server_not_started);
            on_error(ec);
            return;
        }
        
        // Cancel the socket socket
        socket().close(ec);

        if (ec)
        {
            on_error(ec);
            return;
        }
        
        // Update the connected flag
        connected_ = false;
        
        // Update sending/receive flag
        receiving_ = false;
        sending_ = false;
        
        // clear buffers
        clear_buffers();
        
        on_disconnected(ec);
        
        // dispatch unregister session
        auto unregister_session_handler = [this]()
        {
            server_->unregister_session(session_id());
        };
        
        server_->service1()->dispatch(unregister_session_handler);
    };
    
    io_service_->dispatch(disconnect_handler);

    return true;
}

void session::try_receive()
{
    SPDLOG_LOGGER_DEBUG(logger_, "webcrown::session::try_receive Trying receive some data. Session id: {}",
                        session_id_);
    asio::error_code ec;
    if (receiving_)
    {
        SPDLOG_LOGGER_DEBUG(logger_, "webcrown::session::try_receive already in receive mode, returning... Session id: {}",
                            session_id_);
        return;
    }

    if(!is_connected())
    {
        ec = make_error(server_error::server_not_started);
        SPDLOG_LOGGER_DEBUG(logger_, "webcrown::session::try_receive Server is not started. Session id: {}",
                            session_id_);
        on_error(ec);
        return;
    }
    
    receiving_ = true;

    auto async_receive_handler = [this](asio::error_code const& ec, std::size_t bytes_size)
    {
        receiving_ = false;

        SPDLOG_LOGGER_DEBUG(logger_, "webcrown::session::try_receive::async_recv_handler bytes receive {}. Session id: {}",
                            bytes_size,
                            session_id_);
        
        // Received some data from the client
        if (bytes_size > 0)
        {
            // Update statistic
            bytes_received_ += bytes_size;
            //server()->add_bytes_received(bytes_size); // TODO: race condition ?

            // Dispatch event
            on_received(receive_buffer_.data(), bytes_size);

            // if the receive buffer is full, so increase its size
            if (receive_buffer_.size() <= bytes_size)
            {
                SPDLOG_LOGGER_DEBUG(logger_, "webcrown::session::try_receive Resizing receive buffer because receive buffer is full. receive_buffer {} - bytes_received {}",
                                    receive_buffer_.size(),
                                    bytes_size);
                receive_buffer_.resize(2 * bytes_size);
            }

            if (!is_connected())
            {
                SPDLOG_LOGGER_DEBUG(logger_, "webcrown::session::try_receive::async_recv_handler Gracefully disconencted. Session id: {}",
                                    session_id_);
                // We manually disconnect the client, so return...
                return;
            }
        }

        // Try to receive again if the session is valid
        if (!ec)
            try_receive();
        else
            disconnect(ec);
    };

    SPDLOG_LOGGER_DEBUG(logger_, "webcrown::session::try_receive receive_buffer_ size: {}", receive_buffer_.size());

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

bool
session::send_async(void const* buffer, size_t size)
{
    asio::error_code ec;

    if (size == 0)
    {
        ec = make_error(session_error::sent_bytes_is_zero);
        SPDLOG_LOGGER_DEBUG(logger_, "webcrown::session::send_async {} bytes to send, returning... Session id: {}",
                            size,
                            session_id_);
        return 0;
    }

    assert((buffer != nullptr) && "Pointer to the buffer should not be null!");
    if (buffer == nullptr)
    {
        ec = make_error(session_error::sent_buffer_is_nullptr);
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
        if (size == 0)
        {
            auto ec = make_error(session_error::sent_bytes_is_zero);
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
session::send_error(asio::error_code ec)
{
    // Skip asio disconnect errors
    if ((ec == asio::error::connection_aborted) ||
        (ec == asio::error::connection_refused) ||
        (ec == asio::error::connection_reset) ||
        (ec == asio::error::eof) ||
        (ec == asio::error::operation_aborted))
        return;

    on_error(ec);
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
