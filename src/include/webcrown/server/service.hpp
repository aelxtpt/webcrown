#pragma once

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <memory>
#include <vector>
#include <spdlog/spdlog.h>

namespace webcrown {
namespace server {

/// This class abstracts the asio::io_service
/// asio::io_service is a central component in the Asio I/O infrastructure.
/// It provides access to the network I/O services of the underlying operating
/// system.
class service : public std::enable_shared_from_this<service>
{
  /// Asio IO services
  std::vector<std::shared_ptr<asio::io_service>> services_;

  /// Asio working threads
  std::vector<std::thread> threads_;

  /// Logger
  std::shared_ptr<spdlog::logger> logger_;

  /// Asio service state
  std::atomic<bool> starded_;
  std::atomic<std::size_t> round_robin_index_;

public:

  /// Initialize Asio service with single or multiple working threads
  ///
  /// \param threads - Working threads count (default is 1)
  /// \param pool - Asio service thread pool flag (default is false)
  explicit service(std::shared_ptr<spdlog::logger> const& logger, uint threads = 1);

  service(service const&) = delete;
  service(service &&) = delete;
  ~service() = default;

  service& operator=(service const&) = delete;
  service& operator=(service&&) = delete;

  /// Start the service
  void start();

  /// Stop the service
  void stop();

  /// Return true if the service manager is started
  bool is_started() const noexcept { return starded_; }

  /// Get the number of the working threads
  std::size_t threads() const noexcept { return threads_.size(); }

  /// Get the next available Asio IO service
  ///
  /// Method will return single Asio IO service using round-robin algorithm
  /// for io-service-per-thread design
  std::shared_ptr<asio::io_service>& asio_service() noexcept
  { return services_[++round_robin_index_ % services_.size()]; }

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

  void worker_thread(std::shared_ptr<asio::io_service> const& io_service);
};
}}
