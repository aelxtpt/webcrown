#pragma once
#include "Server/Service.hpp"
#include "Server/Http/HttpServer.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>


namespace webcrown {

class WebCrown
{
  std::shared_ptr<server::Service> service_;
  std::shared_ptr<server::http::HttpServer> server_;

  std::shared_ptr<spdlog::logger> logger_;
public:

  explicit WebCrown(
    std::string_view address,
    uint16_t port_number,
    std::shared_ptr<asio::ssl::context> const& context)
  {
    initialize_logger();

    service_ = std::make_shared<server::Service>(logger_);
    server_ = std::make_shared<server::http::HttpServer>(
          logger_,
          service_,
          port_number,
          std::move(address),
          context);
  }

  WebCrown(WebCrown const&) = delete;
  WebCrown(WebCrown&&) = delete;

  WebCrown& operator=(WebCrown const&) = delete;
  WebCrown& operator=(WebCrown&&) = delete;

  virtual ~WebCrown() = default;

  std::shared_ptr<server::Service>& service() noexcept { return service_; }
  std::shared_ptr<server::http::HttpServer>& server() noexcept { return server_; }


  virtual void start()
  {
    service_->start();

    while (!service_->is_started())
    {
      pthread_yield();
    }

    server_->start();

    while (!server_->is_started())
    {
      pthread_yield();
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
    console_sink->set_pattern("%d/%m/%Y %H:%M:%S.%e.%F [%^%l%$] [thread %t] %v");

    logger_.reset(new spdlog::logger("multi_sink", { console_sink}));
    logger_->set_level(spdlog::level::debug);
  }
};

}
