#include "webcrown/server/service.hpp"
#include "webcrown/server/server.hpp"
#include "webcrown/server/service.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <webcrown/server/http/middlewares/routing_middleware.hpp>

#include <chrono>
#include <thread>
#include <stdlib.h>
#include <fstream>

namespace http = webcrown::server::http;

namespace webcrown {

class simple_server : public server::server
{
public:
    explicit simple_server(
        std::shared_ptr<spdlog::logger> logger,
        std::shared_ptr<webcrown::server::service> const& service,
        uint16_t port_num,
        std::string_view address)
        : server::server(logger, service, port_num, address)
    {

    }

    std::shared_ptr<webcrown::server::session> create_session(
      uint64_t session_id,
      std::shared_ptr<server> const& server,
      std::shared_ptr<spdlog::logger> const& logger) override;
};

class simple_session : public webcrown::server::session {
    std::shared_ptr<spdlog::logger> logger_;
public:
    explicit simple_session(
      uint64_t session_id,
      std::shared_ptr<simple_server> server,
      std::shared_ptr<spdlog::logger> const& logger)
        : session(session_id, server, logger)
        , logger_(logger)
    {}

    void on_received(void const* buffer, std::size_t size) override
    {
        logger_->info("Received bytes: {}", size);
    }
};

std::shared_ptr<webcrown::server::session> simple_server::create_session(
  uint64_t session_id,
  std::shared_ptr<server> const& server,
  std::shared_ptr<spdlog::logger> const& logger)
{
    auto session = std::make_shared<simple_session>(
          session_id,
          std::dynamic_pointer_cast<simple_server>(server),
          logger);

    return session;
}

class webcrown_simple
{
    std::shared_ptr<server::service> service_;
    std::shared_ptr<simple_server> server_;

    std::shared_ptr<spdlog::logger> logger_;
public:

    explicit webcrown_simple(
        std::string_view address,
        uint16_t port_number)
    {
        initialize_logger();

        service_ = std::make_shared<server::service>(logger_);
        server_ = std::make_shared<simple_server>(
            logger_,
            service_,
            port_number,
            std::move(address));
    }

    std::shared_ptr<server::service>& service() noexcept { return service_; }
    std::shared_ptr<simple_server>& server() noexcept { return server_; }

    [[nodiscard]] std::shared_ptr<spdlog::logger> logger() const noexcept { return logger_; }

    virtual void start()
    {
        service_->start();

        while (!service_->is_started())
        {
#ifdef __APPLE__
            sched_yield();
#else
            pthread_yield();
#endif
        }

        server_->start();

        while (!server_->is_started())
        {
#ifdef __APPLE__
            sched_yield();
#else
            pthread_yield();
#endif
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

int main()
{
    webcrown::webcrown_simple http("127.0.0.1", 8116);

    http.start();

    for(;;)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    http.stop();
}
