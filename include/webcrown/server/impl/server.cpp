#include "webcrown/server/server.hpp"
#include <assert.h>

namespace webcrown {
namespace server {

server::server(
    std::shared_ptr<spdlog::logger> logger,
    std::shared_ptr<webcrown::server::service> const& service,
    uint16_t port_num,
    std::string_view address)
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
        , last_generated_session_id_(0)
{
}

bool server::start()
{
    logger_->info("[server][start] Starting Server");

    assert(!is_started() && "Server is already started");
    if (is_started())
    {
        logger_->error("[server][start] Server is already started");
        return false;
    }

    auto self(this->shared_from_this());
    auto start_handler = [this, self]()
    {
        if (is_started())
        {
            logger_->error("[server][start][start_handler] Server is already started");
            return;
        }

        asio::error_code ec;
        asio::ip::tcp::endpoint endpoint(asio::ip::make_address(address_, ec), port_number_);

        if (ec.value())
        {
            logger_->error("[server][start][start_handler] Error on make ip address. Code: {}. Message: {}",
                           ec.value(),
                           ec.message());
            return;
        }

        // Open socket
        socket_acceptor_.open(endpoint.protocol(), ec);

        if (ec.value())
        {
            logger_->error("[server][start][start_handler] Error on open acceptor socket. Code: {}. Message: {}",
                     ec.value(),
                     ec.message());

            assert(ec.value() == 0 && "Error on open acceptor socket.");
            return;
        }

        // Bind endpoint
        socket_acceptor_.bind(endpoint, ec);
        if (ec.value())
        {
            logger_->error("[server][start][start_handler] Error on bind acceptor socket. Code: {}. Message: {}",
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

    // post start hander
    io_service_->post(start_handler);

    return true;
}

void server::accept()
{
    assert(is_started() && "Server is not started");
    if (!is_started())
    {
        logger_->warn("[server][accept] Server is not started");
        return;
    }

    logger_->info("[server][accept] Initializing the server accept");
    
    // Dispatch the accept handler
    auto self = shared_from_this();
    auto accept_handler = [this, self]()
    {
        if (!is_started())
        {
            logger_->error("[server][accept][accept_handler] Server is not started");
            return;
        }

        // create new session to accept
        // the earlier session is stored in sessions_. It is not lose, because
        // is a shared_ptr.
        auto session = create_session(++last_generated_session_id_, self, logger_);

        logger_->info("[server][accept][accept_handler] Session {} was created",
            session->session_id());

        auto async_accept_handler = [this, self, session](std::error_code ec)
        {
            if (ec)
            {
                logger_->error("[server][accept][accept_handler][async_accept_handler] Error on async_accept. Code: {}. Message: {}",
                           ec.value(),
                           ec.message());
                
                // TODO: Send error ?
                
                return;
            }

            // Register a new session
            register_session(session);

            // Connect a new session
            session->connect();

            // Perform the next server accept
            accept();
        };

        // Waiting for the next request
        logger_->info("[server][accept][accept_handler] Waiting for the next request...");
        socket_acceptor_.async_accept(
                                      session->socket(),
                                      async_accept_handler);
    };

    // Dispatch accept handler
    io_service_->dispatch(accept_handler);
}

std::shared_ptr<session>
server::create_session(
    uint64_t session_id,
    std::shared_ptr<server> const& server,
    std::shared_ptr<spdlog::logger> const& logger)
{
    logger_->info("[session][create_session] creating session {}", session_id);
    return std::make_shared<session>(session_id, server, logger);
}

void server::register_session(std::shared_ptr<session> s)
{
    std::unique_lock<std::shared_mutex> m(sessions_lock_);
    
    logger_->info("[server][register_session] registering session {}", s->session_id());

    // Register a new session
    sessions_.emplace(s->session_id(), s);
}

void server::unregister_session(uint64_t id)
{
    std::unique_lock<std::shared_mutex> m(sessions_lock_);

    // Try to find the given session
    auto it = sessions_.find(id);
    if (it != sessions_.end())
    {
        logger_->info("[server][unregister_session] unregistering session {}", it->second->session_id());
        
        // Erase the session
        sessions_.erase(it);
    }
}

}}
