#include "webcrown/server/server.hpp"
#include "webcrown/server/error.hpp"
#include <assert.h>

namespace webcrown {
namespace server {

server::server(
    std::shared_ptr<webcrown::server::service> const& service,
    uint16_t port_num,
    std::string_view address)
        : started_(false)
        , socket_acceptor_(*service->asio_service())
        , io_service_(service->asio_service())
        , service_(service)
        , bytes_pending_(0)
        , bytes_sent_(0)
        , bytes_received_(0)
        , address_(address)
        , port_number_(port_num)
        , last_generated_session_id_(0)
{
}

void server::on_started() {}

void server::start()
{
    assert(!is_started() && "Server is already started");
    if (is_started())
    {
        auto ec = make_error(server_error::server_already_started);
        on_error(ec);
        return;
    }

    auto start_handler = [this]()
    {
        asio::error_code ec;
        asio::ip::tcp::endpoint endpoint(asio::ip::make_address(address_, ec), port_number_);

        if (ec.value())
        {
            assert(ec.value() == 0 && "Error on make endpoint");
            on_error(ec);
            return;
        }

        socket_acceptor_.open(endpoint.protocol(), ec);

        if (ec.value())
        {
            assert(ec.value() == 0 && "Error on open acceptor socket.");
            on_error(ec);
            return;
        }

        socket_acceptor_.bind(endpoint, ec);
        if (ec.value())
        {
            assert(ec.value() == 0 && "Error on bind acceptor socket.");
            on_error(ec);
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
        on_started();

        // Perform the first server accept
        accept();
    };

    service1()->dispatch(start_handler);
}

void server::accept()
{
    asio::error_code ec;
    assert(is_started() && "Server is not started");
    if (!is_started())
    {
        ec = make_error(server_error::server_not_started);
        return;
    }
    
    // Dispatch the accept handler
    auto self = shared_from_this();
    auto accept_handler = [this, self, &ec]()
    {
        if (!is_started())
        {
            ec = make_error(server_error::server_not_started);
            on_error(ec);
            return;
        }

        // create new session to accept
        // the earlier session is stored in sessions_. It is not lose, because
        // is a shared_ptr.
        auto session = create_session(++last_generated_session_id_, self);

        auto async_accept_handler = [this, self, session](std::error_code ec)
        {
            if (ec)
            {
                on_error(ec);
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
        socket_acceptor_.async_accept(session->socket(),
                                      async_accept_handler);
    };

    // Dispatch accept handler
    io_service_->dispatch(accept_handler);
}

std::shared_ptr<session>
server::create_session(
    uint64_t session_id,
    std::shared_ptr<server> const& server)
{
    return std::make_shared<session>(session_id, server);
}

void server::register_session(std::shared_ptr<session> s)
{
    std::unique_lock<std::shared_mutex> m(sessions_lock_);

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
        // Erase the session
        sessions_.erase(it);
    }
}

}}
