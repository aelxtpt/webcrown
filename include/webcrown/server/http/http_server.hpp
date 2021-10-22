#pragma once
#include "webcrown/server/server.hpp"
#include "webcrown/server/service.hpp"
#include "webcrown/server/http/middlewares/http_middleware.hpp"

namespace webcrown {
namespace server {
namespace http {

class http_server : public server
{
    /// This is an object representing SSL context. Basically
    /// this is a wrapper around the SSL_CTX data structure defined
    /// by OpenSSL library
    //std::shared_ptr<asio::ssl::context> context_;

    std::vector<std::shared_ptr<middleware>> middlewares_;
public:
    explicit http_server(
            std::shared_ptr<webcrown::server::service> const& service,
            uint16_t port_num,
            std::string_view address);

    http_server(http_server const&) = delete;
    http_server(http_server&&) = delete;

    http_server& operator=(http_server const&) = delete;
    http_server& operator=(http_server&&) = delete;

    ~http_server() = default;

    void add_middleware(std::shared_ptr<middleware> const middleware);

    void on_error(asio::error_code &ec) override;
    void on_started() override;
private:
    std::shared_ptr<session> create_session(
        uint64_t session_id,
        std::shared_ptr<server> const& server) override;

};

}}}
