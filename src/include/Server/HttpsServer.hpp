#pragma once
#include "Server/SslServer.hpp"

namespace webcrown {
namespace server {

class HttpServer : public SslServer
{
public:


  // SslServer interface
private:
  std::shared_ptr<SslSession> create_session(
      uint64_t session_id, std::shared_ptr<SslServer> const& server, std::shared_ptr<spdlog::logger> const& logger) override;
};

}}
