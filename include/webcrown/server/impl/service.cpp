
#include "webcrown/server/service.hpp"
#include "webcrown/server/error.hpp"

namespace webcrown {
namespace server {

service::service(bool use_pool, uint threads)
  : started_(false)
  , round_robin_index_(0)
{
    assert(threads <= 1 && "Threads count cannot be zero");

    logger_ = spdlog::get(webcrown::logger_name);
    if(!logger_)
    {
        // register default logger sinks
        std::vector<spdlog::sink_ptr> sinks;

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::debug);
        console_sink->set_pattern("%d/%m/%Y %H:%M:%S.%e.%f.%F %u [%^%l%$] [thread %t] %v");

        sinks.push_back(console_sink);

        webcrown::setup_logger(sinks);

        logger_ = spdlog::get(webcrown::logger_name);
    }

    for (uint i = 0; i < threads; ++i)
    {
        services_.emplace_back(std::make_shared<asio::io_service>());

        // Without this, the code will be executed in the caller thread
        // We will create the a thread to each service
        threads_.emplace_back(std::thread());
    }
}

void service::start(asio::error_code &ec)
{
    assert(!is_started() && "Service is already started");
    if (is_started())
    {
        ec = make_error(service_error::service_not_started);
        return;
    }

    SPDLOG_LOGGER_DEBUG(logger_, "webcrown::service::start Starting service: threads size: {}",
                 threads_.size());

    // Reset round robin index
    round_robin_index_ = 0;

    auto start_handler = [this]()
    {
        if (is_started())
        {
            return;
        }

        started_ = true;
    };

    // We need post the job in any io_service. Why?
    // Because we want execute the content of the start handler
    // when the io_service::run execute the jobs
    // We only need of ONE service to execute this specific start handler
    // Because it only print message and set started to true
    services_[0]->post(start_handler);

    for (std::size_t i = 0; i < threads_.size(); ++i)
    {
        threads_[i] = std::thread([this, i]()
        {
            worker_thread(services_[i]);
        });
    }
}

void service::stop(asio::error_code &ec)
{
    assert(is_started() && "Service is not started");
    if (!is_started())
    {
        ec = make_error(service_error::service_not_started);
        return;
    }

    auto stop_handler = [this]()
    {
        if (!is_started())
        {
            return;
        }

        // Stop Asio services
        for (auto& service : services_)
            service->stop();

        // Update the started flag
        started_ = false;
    };

    services_[0]->post(stop_handler);

    // Wait for all services working threads
    for (auto& thread : threads_)
    {
        thread.join();
    }
}

void service::worker_thread(std::shared_ptr<asio::io_service> const& io_service)
{
    try
    {
        // Attach the current working thread to the Asio service
        // The work class is used to inform the io_context when work starts and
        // * finishes. This ensures that the io_context object's run() function will not
        // * exit while work is underway, and that it does exit when there is no
        // * unfinished work remaining.
        asio::io_service::work worker(*io_service);

        // worker loop
        do
        {
            // Blocks execution while there are unfinished asynchronous operations.
            // While inside the call to io_service::run(), the I/O execution context dequeue
            // the result of the operation, translates it into error_code, and then passes it to your completion handler.
            io_service->run();
        }
        while (is_started());
    }
    catch(asio::system_error const& ex)
    {
        // TODO: Trait this error with upload multipart
        // curl -v -X POST http://localhost:8080/upload \
        // -F "upload[]=@/Users/alex/GolandProjects/multipart/avatar_avatar.jpg" \
        // -H "Content-Type: multipart/form-data"

        throw ex;
    }
    catch(std::exception const& ex)
    {
        printf("Exception %s\n", ex.what());
        //throw ex;
    }

  // TODO: call the cleanup thread handler
}

}}
