#pragma once

#include "Server/Http/HttpContext.hpp"

#include "Server/Http/HttpEndpoint.hpp"

#include <memory>
#include <string>
#include <functional>



namespace webcrown {
namespace server {
namespace http {

// TODO: Aplicar os patterns
struct IController
{
  virtual ~IController() {}

  virtual void on_setup(std::shared_ptr<HttpContext> const& context) = 0;

  virtual std::vector<std::shared_ptr<HttpEndpoint>> endpoints() const = 0;
};

}}}
