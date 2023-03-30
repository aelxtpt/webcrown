#pragma once

#include <asio.hpp>
#include <memory>
#include <vector>

//#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include "webcrown/logger/webcrown_logger.hpp"


namespace webcrown {
namespace server {

using std::vector;
using std::shared_ptr;
using std::atomic;

/**
 * @brief This class abstracts the asio::io_service
 * asio::io_service is a central component in the Asio I/O infrastructure.
 * It provides access to the network I/O services of the underlying operating system.
 */
class service : public std::enable_shared_from_this<service>
{
    /**
     * @brief Asio services
     */
    vector<shared_ptr<asio::io_service>> services_;

    /**
     * @brief Asio working threads
     */
    vector<std::thread> threads_;

    /**
     * @brief Asio server state
     */
    atomic<bool> started_;
    atomic<std::size_t> round_robin_index_;

    shared_ptr<spdlog::logger> logger_;
public:
    /**
     * @brief Initialize Asio service with single or multiple working threads
     * @param use_pool - Asio service thread pool flag (defualt is false)
     * @param threads - Working thread count (default is 1)
     */
    explicit service(bool use_pool = false, uint threads = 1);

    service(service const&) = delete;
    service(service &&) = delete;
    ~service() = default;

    service& operator=(service const&) = delete;
    service& operator=(service&&) = delete;

    /**
     * @brief start the service
     * @param ec
     */
    void start(asio::error_code& ec);

    /// Stop the service
    void stop(asio::error_code& ec);

    /// Return true if the service manager is started
    bool is_started() const noexcept { return started_; }

    /// Get the number of the working threads
    std::size_t threads() const noexcept { return threads_.size(); }

    /// Get the next available Asio IO service
    ///
    /// Method will return single Asio IO service using round-robin algorithm
    /// for io-service-per-thread design
    shared_ptr<asio::io_service>& asio_service() noexcept
    { return services_[++round_robin_index_ % services_.size()]; }

    decltype(logger_) logger() const noexcept { return logger_; }

    /// Post the given handler
    ///
    /// The given handler will be enqueued to the IO service pending operations queue.
    template<typename CompletionHandler>
    void post(CompletionHandler&& handler)
    { services_[0]->post(handler); }

    template<typename CompletionHandler>
    void dispatch(CompletionHandler&& handler)
    { services_[0]->dispatch(handler); }

private:

    void worker_thread(shared_ptr<asio::io_service> const& io_service);
};
}}
