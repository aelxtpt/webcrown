#include "webcrown/server/server.hpp"
#include <assert.h>


namespace webcrown {
namespace server {

server::server(
    std::shared_ptr<spdlog::logger> logger,
    std::shared_ptr<webcrown::server::service> const& service,
    uint16_t port_num,
    std::string_view address,
    std::shared_ptr<asio::ssl::context> const& context)
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
        , context_(context)
        , last_generated_session_id_(0)
{
}

bool server::start()
{
    logger_->info("[Server][start] Starting SSL Server");

    assert(!is_started() && "Server is already started");
    if (is_started())
    {
        logger_->error("[SslServer][start] SSL Server is already started");
        return false;
    }

    // Post the start handler
    auto start_handler = [this]()
    {
        if (is_started())
        {
            logger_->error("[SslServer][start][start_handler] SSL Server is already started");
            return;
        }

        asio::error_code ec;
        asio::ip::tcp::endpoint endpoint(asio::ip::make_address(address_, ec), port_number_);

        if (ec.value())
        {
            logger_->error("[SslServer][start][start_handler] Error on make ip address. Code: {}. Message: {}",
                           ec.value(),
                           ec.message());
            return;
        }

        // Open socket
        socket_acceptor_.open(endpoint.protocol(), ec);

        if (ec.value())
        {
            logger_->error("[SslServer][start][start_handler] Error on open acceptor socket. Code: {}. Message: {}",
                     ec.value(),
                     ec.message());

            assert(ec.value() == 0 && "Error on open acceptor socket.");
            return;
        }

        // Bind endpoint
        socket_acceptor_.bind(endpoint, ec);
        if (ec.value())
        {
            logger_->error("[SslServer][start][start_handler] Error on bind acceptor socket. Code: {}. Message: {}",
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

void server::accept()
{
    logger_->info("[Server][accept] Initializing the server accept");

    assert(is_started() && "Server is not started");
    if (!is_started())
    {
        logger_->error("[Server][accept] SSL Server is not started");
        return;
    }

    // Dispatch the accept handler
    auto self = shared_from_this();
    auto accept_handler = [this, self]()
    {
        if (!is_started())
        {
            logger_->error("[Server][accept][accept_handler] Server is not started");
            return;
        }

        // create new session to accept
        // the earlier session is stored in sessions_. It is not lose, because
        // is a shared_ptr.
        session_ = create_session(++last_generated_session_id_, self, logger_);

        logger_->info("[Server][accept][accept_handler] Session {} was created",
            session_->session_id());

        auto async_accept_handler = [this, self](std::error_code ec)
        {
            if (!is_started())
            {
                logger_->error("[Server][accept][accept_handler][async_accept_handler] SSL Server is not started");
                return;
            }

            if (ec)
            {
                logger_->error("[Server][accept][accept_handler][async_accept_handler] Error on async_accept. Code: {}. Message: {}",
                           ec.value(),
                           ec.message());
                return;
            }

            // Register a new session
            register_session();

            // Connect a new session
            session_->connect();

            // Perform the next server accept
            accept();
        };

        // Waiting for the next request
        logger_->info("[Server][accept][accept_handler] Waiting for the next request...");
        socket_acceptor_.async_accept(session_->socket(), async_accept_handler);
    };

    // Dispatch accept handler
    io_service_->dispatch(accept_handler);
}

std::shared_ptr<session> server::create_session(
    uint64_t session_id,
    std::shared_ptr<server> const& server,
    std::shared_ptr<spdlog::logger> const& logger)
{
    return std::make_shared<session>(session_id, server, logger, context_);
}

void server::register_session()
{
    std::unique_lock<std::shared_mutex> m(sessions_lock_);

    // Register a new session
    sessions_.emplace(session_->session_id(), session_);
}

void server::unregister_session(uint64_t id)
{
    std::unique_lock<std::shared_mutex> m(sessions_lock_);

    // Try to find the given session
    auto it = sessions_.find(id);
    if (it != sessions_.end())
    {
        // Erase the session
        sessions_.erase(it);
    }
}

}}
