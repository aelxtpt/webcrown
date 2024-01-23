#include "webcrown/server/webserver.hpp"
#include "asio/error_code.hpp"
#include "asio/io_context.hpp"
#include "asio/socket_base.hpp"
#include "asio/steady_timer.hpp"
#include "webcrown/server/error.hpp"
#include <thread>

namespace webcrown {
namespace server {

WebSession::WebSession(uint64_t session_id, shared_ptr<WebServer> server, OnCb& cb)
    : session_id_(session_id)
    , server_(server)
    , io_context_(server->io_context_)
    , socket_(*server->io_context_)
    , on_error_(cb)
{

}

WebSession::~WebSession()
{
    int brk = 0;
}

void
WebSession::connect()
{
    bytes_pending_ = 0;
    bytes_sending_ = 0;
    bytes_received_ = 0;
    bytes_sent_ = 0;

    receive_buffer_.resize(option_receive_buffer_size());

    connected_ = true;

    try_receive();
}

void
WebSession::clear_buffers()
{
    std::scoped_lock locker(send_lock_);

    // Clear send buffers
    send_buffer_main_.clear();
    send_buffer_flush_.clear();

    // Update statistic
    bytes_pending_ = 0;
    bytes_sending_ = 0;
}

bool
WebSession::disconnect(asio::error_code ec)
{
    auto shutdown_session = [this]()
    {
        asio::error_code ec;
        // Indicates that this session will no more receives data
        socket_.shutdown(asio::socket_base::shutdown_receive, ec);
        if(ec)
            on_error_(ec);

        socket_.close();
    };

    if(!connected_)
    {
        ec = make_error(server_error::server_not_started);
        on_error_(ec);
        shutdown_session();
        return false;
    }

    auto disconnect_handler = [this, &shutdown_session]()
    {
        asio::error_code ec;
        if(!connected_)
        {
            ec = make_error(server_error::server_not_started);
            shutdown_session();
            return;
        }

        // Cancel the socket
        auto _ = socket_.close(ec);
        if(ec)
        {
            shutdown_session();
            on_error_(ec);
            return;
        }

        connected_ = false;

        // Update sending/receive flag
        receiving_ = false;
        sending_ = false;

        // clearbuffers
        clear_buffers();

        shutdown_session();

        auto unregister_session_handler = [this]()
        {
            server_->unregister_session(session_id_);
        };

        server_->io_context_->dispatch(unregister_session_handler);
    };

    io_context_->dispatch(disconnect_handler);

    return true;
}

void
WebSession::try_receive()
{
    asio::error_code ec;
    if(receiving_)
    {
        return;
    }

    if(!connected_)
    {
        ec = make_error(session_error::not_connected);
        on_error_(ec);
        return;
    }

    receiving_ = true;

    auto async_receive_handler = [this](asio::error_code const& ec, std::size_t bytes_size)
    {
        receiving_ = false;

        if(ec)
        {
            disconnect(ec);
            return;
        }

        if(bytes_size == 0)
        {
            try_receive();
            return;
        }

        bytes_received_ += bytes_size;

        // Dispatch event
        on_receive(receive_buffer_.data(), bytes_size);

        // receive buffer is full
        if(receive_buffer_.size() <= bytes_size)
            receive_buffer_.resize(2 * bytes_size);

        if(!connected_)
        {
            // we manually disconnect the client, so return
            return;
        }

        // Receive next buffer
        try_receive();
    };

    socket_.async_read_some(
        asio::buffer(receive_buffer_.data(), receive_buffer_.size()),
        async_receive_handler
    );
}

void
WebSession::on_receive(void const* buffer, std::size_t size)
{
    std::error_code ec;

    // parser
    auto result = parser_.parse(static_cast<const char*>(buffer), size, ec);
    if (ec)
    {
        //logger_->error("[http_session][on_received] failed to parser start line {}", ec.message());
        disconnect(ec);
        return;
    }

    if (parser_.parsephase() != http::parse_phase::finished)
    {
        // need more
        //logger_->info("[http_session][on_received] need more bytes...");
        return;
    }

    if (!result)
    {
        //logger_->error("[http_session][on_received] no http request");
        disconnect(ec);
        return;
    }

    http::http_response response{};
    // middlewares
    for(auto& middleware : server_->middlewares_)
    {
        if(!middleware->execute(*result, response))
        {
            break;
        }
    }

    // send response
    send_async(response.build());
    //logger_->info("[http_session][on_received] Message sent to the client");

    // disconnect 
    if(!disconnect())
    {
        //logger_->error("[http_session][on_received] Failed to disconnect session.");
    }
}

std::size_t 
WebSession::option_receive_buffer_size() const
{
    asio::socket_base::receive_buffer_size option;
    socket_.get_option(option);

    return option.value();
}

bool
WebSession::send_async(void const* buffer, size_t size)
{
    asio::error_code ec;

    if(size == 0)
    {
        ec = make_error(session_error::sent_bytes_is_zero);
        disconnect();
        return false;
    }

    assert((buffer != nullptr) && "Pointer to the buffer should no be null!");
    if(buffer == nullptr)
    {
        ec = make_error(session_error::sent_buffer_is_nullptr);
        disconnect();
        return 0;
    }

    {
        std::scoped_lock locker(send_lock_);

        // Detect multiple send handlers
        auto send_required = send_buffer_main_.empty() || send_buffer_flush_.empty();

        // Fill the main send buffer
        uint8_t const* bytes = (uint8_t const*)buffer;
        send_buffer_main_.insert(send_buffer_main_.end(), bytes, bytes + size);

        // Update Statistics
        bytes_pending_ = send_buffer_main_.size();

        // Avoid multiple send handlers
        if(!send_required)
            return true;
    }

    auto send_handler = [this]()
    {
        try_send();
    };

    io_context_->dispatch(send_handler);
    return true;
}

void
WebSession::try_send()
{
    if(sending_)
        return;

    // Swap send buffers
    if(send_buffer_flush_.empty())
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
    if(send_buffer_flush_.empty())
    {
        // Call the empty send buffer handler
        // on empty();
        return;
    }

    // Async write with the write handler
    sending_ = true;
    auto self = shared_from_this();
    auto async_write_handler = [this, self](std::error_code ec, size_t size)
    {
        sending_ = false;

        if(size == 0)
        {
            auto ec = make_error(session_error::sent_bytes_is_zero);
            return;
        }

        // Update statistics
        bytes_sending_ -= size;
        bytes_sent_ += size;

        // Increase the flush buffer offset
        send_buffer_flush_offset += size;

        // Successfully send the whole flush buffer
        if (send_buffer_flush_offset == send_buffer_flush_.size())
        {
            // Clear the flush buffer
            send_buffer_flush_.clear();
            send_buffer_flush_offset = 0;
        }

        // Try to send again if the session is valid
        if(!ec)
            try_send();
        else
        {
            //send_error(ec);
            disconnect(ec);
        }
    };

    socket_.async_write_some(
        asio::buffer(send_buffer_flush_.data() + send_buffer_flush_offset,
        send_buffer_flush_.size() - send_buffer_flush_offset),
        async_write_handler
    );
}

void
WebSession::send_error(asio::error_code ec)
{
    // Skip asio disconnect errors
    if ((ec == asio::error::connection_aborted) ||
        (ec == asio::error::connection_refused) ||
        (ec == asio::error::connection_reset) ||
        (ec == asio::error::eof) ||
        (ec == asio::error::operation_aborted))
        return;

    //on_error(ec);
}

// ---------------------------------------------------------------------------

WebServer::WebServer(
        std::string host,
        uint16_t port,
        OnCb const& cb)
    : started_(false)
    , last_session_id_(0)
    , io_context_(std::make_shared<asio::io_context>())
    , socket_acceptor_(*io_context_)
    , on_error_(cb)
    , host_(std::move(host))
    , port_(port)
{
    
}

WebServer::~WebServer()
{
    if(context_worker_thread_.joinable())
        context_worker_thread_.join();
}

void 
WebServer::start()
{
    asio::error_code ec;
    assert(!started_ && "Webserver is already started");
    
    started_ = true;

    // Run I/O thread 
    context_worker_thread_ = std::thread(&WebServer::context_handler, this);

    auto start_handler = [this]()
    {
        asio::error_code ec;
        asio::ip::tcp::endpoint endpoint(asio::ip::make_address(host_, ec), port_);
        
        if(ec.value())
        {
            on_error_(ec);
            return;
        }

        auto _ = socket_acceptor_.open(endpoint.protocol(), ec);
        if (ec.value())
        {
            on_error_(ec);
            return;
        }

        _ = socket_acceptor_.bind(endpoint, ec);
        if(ec.value())
        {
            on_error_(ec);
            return;
        }

        socket_acceptor_.listen();

        // Perform first server accept
        accept();
    };

    io_context_->dispatch(start_handler);
}

void
WebServer::accept()
{
    asio::error_code ec;
    assert(started_ && "Server it not started");
    if(!started_)
    {
        ec = make_error(server_error::server_not_started);
        on_error_(ec);
        return;
    }

    auto accept_handler = [this, &ec]()
    {
        if(!started_)
        {
            ec = make_error(server_error::server_not_started);
            on_error_(ec);
            return;
        }

        auto session_id = ++last_session_id_;
        auto session = create_session(session_id);

        auto async_accept_handler = [this, session_id, session](std::error_code ec)
        {
            if(ec)
            {
                on_error_(ec);
                return;
            }

            register_session(session);
            session->connect();

            auto disconnect_session = [session_id, session](asio::error_code ec)
            {
                if(!session->is_connected())
                    return;

                session->disconnect();
            };

            // Expire session
            asio::steady_timer expire_session_t(*io_context_);
            expire_session_t.expires_after(std::chrono::seconds(3));
            expire_session_t.async_wait(disconnect_session);


            // Next server accept
            accept();
        };

        socket_acceptor_.async_accept(session->socket(), async_accept_handler);
    };

    io_context_->dispatch(accept_handler);
}

void
WebServer::stop()
{
    asio::error_code ec;
    assert(!started_ && "Service is not started");
    if(!started_)
    {
        ec = make_error(service_error::service_not_started);
        on_error_(ec);
        return;
    }

    started_ = false;

    if(context_worker_thread_.joinable())
        context_worker_thread_.join();
}

shared_ptr<WebSession>
WebServer::create_session(uint64_t session_id)
{
    auto self = shared_from_this();
    return std::make_shared<WebSession>(session_id, self, on_error_);
}

void
WebServer::register_session(shared_ptr<WebSession> session)
{
    std::unique_lock<std::shared_mutex> m(sessions_lock_);

    sessions_.emplace(session->session_id(), session);
}

void
WebServer::unregister_session(uint64_t id)
{
    std::unique_lock<std::shared_mutex> m (sessions_lock_);

    auto it = sessions_.find(id);
    if(it != sessions_.end())
    {
        sessions_.erase(it);
    }
}

void
WebServer::add_middleware(shared_ptr<http::middleware> const middleware)
{
    middlewares_.push_back(middleware);
}

void
WebServer::context_handler()
{
    try
    {
        asio::io_context::work worker(*io_context_);

        do
        {
            // Blocks execution while there are unfinished asynchronous operations.
            // While inside the call to io_context::run(), the I/O execution context dequeue
            // the result of the operation, translates it into error_code, and then passes it to your completion handler.
            io_context_->run();
        }
        while(started_);
    }
    catch(std::exception const& ex)
    {

    }
}

}
}
