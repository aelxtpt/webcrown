#pragma once
#include "webcrown/server/service.hpp"
#include "webcrown/server/http/http_server.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace webcrown {

class webcrown_http
{
  std::shared_ptr<server::service> service_;
  std::shared_ptr<server::http::http_server> server_;

  std::shared_ptr<spdlog::logger> logger_;
public:

  explicit webcrown_http(
      std::string_view address,
      uint16_t port_number,
      std::shared_ptr<asio::ssl::context> const& context)
  {
    initialize_logger();

    service_ = std::make_shared<server::service>(logger_);
    server_ = std::make_shared<server::http::http_server>(
          logger_,
          service_,
          port_number,
          std::move(address),
          context);
  }

  webcrown_http(webcrown_http const&) = delete;
  webcrown_http(webcrown_http&&) = delete;

  webcrown_http& operator=(webcrown_http const&) = delete;
  webcrown_http& operator=(webcrown_http&&) = delete;

  virtual ~webcrown_http() = default;

  std::shared_ptr<server::service>& service() noexcept { return service_; }
  std::shared_ptr<server::http::http_server>& server() noexcept { return server_; }

  virtual void start()
  {
    service_->start();

    while (!service_->is_started())
    {
      pthread_yield();
      //sched_yield();
    }

    server_->start();

    while (!server_->is_started())
    {
        pthread_yield();
        //sched_yield();
    }
  }

  virtual void stop()
  {
    service_->stop();
  }
private:

  /// Initialize the spd logger context
  virtual void initialize_logger()
  {
    spdlog::set_pattern("[%H:%M:%S %z] [%^%L%$] [thread %t] %v");
    spdlog::set_level(spdlog::level::debug);

    // Flush all *registered* loggers using a worker thread every 1 seconds.
    // note: registered loggers *must* be thread safe for this to work correctly!
    spdlog::flush_every(std::chrono::seconds(1));

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::debug);
    console_sink->set_pattern("%d/%m/%Y %H:%M:%S.%e.%f.%F %u [%^%l%$] [thread %t] %v");

    logger_.reset(new spdlog::logger("multi_sink", { console_sink}));
    logger_->set_level(spdlog::level::debug);
  }
};

}
