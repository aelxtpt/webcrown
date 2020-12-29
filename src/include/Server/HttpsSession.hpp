#pragma once
#include "Server/SslSession.hpp"

namespace webcrown {
namespace server {

class HttpsSession : public SslSession
{
public:
  void on_receive(void const* buffer, std::size_t size) override;
};

}}
