#pragma once
#include "webcrown/server/session.hpp"
#include <memory>
#include <shared_mutex>
#include <map>

#include <asio/error_code.hpp>

namespace webcrown {
namespace server {

/**
 * @brief      This class describes a server.
 */
class server: public std::enable_shared_from_this<server>
{
    std::atomic<bool> started_;

    asio::ip::tcp::acceptor socket_acceptor_;
    std::shared_ptr<asio::io_service> io_service_;

    // Asio Service
    std::shared_ptr<service> service_;

    // Server statistics
    uint64_t bytes_pending_;
    uint64_t bytes_sent_;
    uint64_t bytes_received_;

    // Server config
    std::string address_;
    uint16_t port_number_;

    // Threading sessions
    std::shared_mutex sessions_lock_;
    std::map<uint64_t, std::shared_ptr<session>> sessions_;
    std::atomic<uint64_t> last_generated_session_id_;
public:
    explicit server(
        std::shared_ptr<service> const& service,
        uint16_t port_num,
        std::string_view address);

    server(server const&) = delete;
    server(server&&) = delete;

    server& operator=(server const&) = delete;
    server& operator=(server&&) = delete;

    virtual ~server() = default;

    /// Start the server
    void start();

    /**
     * @brief Check if the server is started
     * @return
     */
    bool is_started() const noexcept { return started_; }

    // Get the Asio Service
    std::shared_ptr<webcrown::server::service> service1() noexcept { return service_; }

    /// Unregister the given session
    ///
    /// \param id - the session id
    void unregister_session(uint64_t id);

    virtual void on_started() = 0;
    virtual void on_error(asio::error_code& ec) = 0;
private:
    void accept();

    /// Create a new session
    ///
    /// \param session_id - unique identifier for the session
    /// \param server - the connected server
    virtual std::shared_ptr<session> create_session(
        uint64_t session_id,
        std::shared_ptr<server> const& server);

    /// Register a new session
    ///
    /// A session is responsible to handle
    /// incoming requests and send response to them
    void register_session(std::shared_ptr<session> s);
};

}}
