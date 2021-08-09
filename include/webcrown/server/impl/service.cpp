#include "webcrown/server/service.hpp"

namespace webcrown {
namespace server {

service::service(const std::shared_ptr<spdlog::logger>& logger, uint threads)
  : logger_(logger)
  , starded_(false)
  , round_robin_index_(0)
{
    assert(threads <= 1 && "Threads count cannot be zero");
    for (uint i = 0; i < threads; ++i)
    {
        services_.emplace_back(std::make_shared<asio::io_service>());

        // Without this, the code will be executed in the caller thread
        // We will create the a thread to each service
        threads_.emplace_back(std::thread());
    }
}

void service::start()
{
    assert(!is_started() && "Service is already started");
    if (is_started())
    {
        logger_->warn("[Service][start][start_handler] Service is already started");
        return;
    }

    // Reset round robin index
    round_robin_index_ = 0;

    auto start_handler = [this]()
    {
        if (is_started())
        {
            logger_->warn("[Service][start][start_handler] Service is already started");
            return;
        }

        logger_->info("[Service][start][start_handler] Service start job was initiated");
        starded_ = true;
    };

    // We need post the job in any io_service. Why?
    // Because we want execute the content of the start handler
    // when the io_service::run execute the jobs
    // We only need of one service to execute this specific start handler
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

void service::stop()
{
    assert(is_started() && "Service is not started");
    if (!is_started())
    {
        logger_->warn("[Service][stop] Service is not started");
        return;
    }

    auto stop_handler = [this]()
    {
        if (!is_started())
        {
            logger_->warn("[Service][stop][stop_handler] Service is not started");
            return;
        }

        // Stop Asio services
        for (auto& service : services_)
            service->stop();

        // Update the started flag
        starded_ = false;
    };

    services_[0]->post(stop_handler);

    // Wait for all services working threads
    for (auto& thread : threads_)
    {
        logger_->info("[Service][stop] Waiting for worker thread...");
        thread.join();
    }
}

void service::worker_thread(std::shared_ptr<asio::io_service> const& io_service)
{
    logger_->info("[Service][worker_thread] Worker thread was started");

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

        logger_->error("[Service][worker_thread] Asio Error on worker loop: {}",
                       ex.what());
    }
    catch(std::exception const& ex)
    {
        logger_->error("[Service][worker_thread] Error on worker loop: {}",
            ex.what());
    }
    catch(...)
    {
        logger_->critical("[Service][worker_thread] Asio worker loop thread terminated!");
    }

  // TODO: call the cleanup thread handler
}

}}
