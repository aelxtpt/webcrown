#pragma once
#include <cstdint>


namespace webcrown {
namespace server {
namespace http {

enum class HttpMethod : std::uint8_t
{
  none = 0,
  get = 1,
  post = 2
};

}}}
