#pragma once
#include "asio/io_context.hpp"
#include "webcrown/server/http/http_parser.hpp"
#include "webcrown/server/http/middlewares/http_middleware.hpp"
#include <asio.hpp>
#include <memory>
#include <shared_mutex>
#include <vector>
#include <map>

namespace webcrown {
namespace server {

using std::vector;
using std::shared_ptr;
using std::atomic;

class WebServer;

class WebSession
{
    using OnCb = std::function<void(asio::error_code ec)>;

    // Statistics
    uint64_t bytes_pending_;
    uint64_t bytes_sending_;
    uint64_t bytes_received_;
    uint64_t bytes_sent_;

    shared_ptr<asio::io_context> io_context_;
    shared_ptr<WebServer> server_;
    asio::ip::tcp::socket socket_;

    atomic<bool> connected_;
    atomic<bool> receiving_;

    uint64_t session_id_;

    vector<uint8_t> receive_buffer_;

    std::mutex send_lock_;

    std::vector<uint8_t> send_buffer_main_;
    std::vector<uint8_t> send_buffer_flush_;
    size_t send_buffer_flush_offset;

    std::atomic<bool> sending_;
    http::parser parser_;
    OnCb& on_error_;
public:
    explicit WebSession(
        uint64_t session_id,
        shared_ptr<WebServer> server,
        OnCb& cb);

    ~WebSession();

    void connect();
    bool disconnect(asio::error_code ec = {});

    bool send_async(void const* buffer, size_t size);
    bool send_async(std::string_view text) { return send_async(text.data(), text.size()); }

    void on_receive(void const* buffer, std::size_t size);

    bool is_connected() const noexcept { return connected_; }

    asio::ip::tcp::socket& socket() noexcept { return socket_; }
    uint64_t session_id() const noexcept { return session_id_; }
private:
    void clear_buffers();

    void try_receive();

    void try_send();

    std::size_t option_receive_buffer_size() const;
    void send_error(asio::error_code ec);
};

class WebServer : public std::enable_shared_from_this<WebServer>
{
    friend class WebSession;
    
    using OnCb = std::function<void(asio::error_code ec)>;

    shared_ptr<asio::io_context> io_context_;

    vector<std::thread> working_threads_;
    std::thread context_worker_thread_;
    atomic<bool> started_;
    atomic<uint64_t> last_session_id_;

    asio::ip::tcp::acceptor socket_acceptor_;
    std::string host_;
    uint16_t port_;

    std::shared_mutex sessions_lock_;
    std::map<uint64_t, shared_ptr<WebSession>> sessions_;

    OnCb on_error_;

    vector<shared_ptr<http::middleware>> middlewares_;
public:
    explicit WebServer(
        std::string host,
        uint16_t port,
        OnCb const& cb);

    WebServer(WebServer const&) = delete;
    WebServer(WebServer &&) = delete;
    ~WebServer();

    WebServer& operator=(WebServer const&) = delete;
    WebServer& operator=(WebServer &&) = delete;

    void start();
    void stop();
     
    bool is_started() const noexcept { return started_; }

    void add_middleware(shared_ptr<http::middleware> const middleware);

    shared_ptr<asio::io_context>& asio_context() noexcept
    { return io_context_; }
private:
    void context_handler();
    void accept();

    shared_ptr<WebSession> create_session(
        uint64_t session_id
    );

    void register_session(shared_ptr<WebSession> s);
    void unregister_session(uint64_t id);
};

} // server
} // webcrown