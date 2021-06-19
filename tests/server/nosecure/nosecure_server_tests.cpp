#include <gmock/gmock.h>
#include <memory>
#include "webcrown/server/service.hpp"
#include "webcrown/server/server.hpp"

namespace server =  webcrown::server;


TEST(NOSECURE_SERVER, xpto)
{
    auto logger = std::make_shared<spdlog::logger>("xpto");
    auto service = std::make_shared<server::service>(logger);

    auto context = std::make_shared<asio::ssl::context>(asio::ssl::context::tlsv12);

    auto server = std::make_shared<server::server<server::SecureSocket>>(
        logger,
        service,
        8433,
        "",
        context
    );

    // thread 1 service and server starts


}