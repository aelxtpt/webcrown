#ifndef WEBCROWN_ENUMS_HPP
#define WEBCROWN_ENUMS_HPP

#include <cstdint>

namespace webcrown {
namespace server {
namespace http {


enum class protocol_version : std::uint8_t
{
    unknown = 0,

    http_1_0,
    http_1_1
};

enum class parse_phase : std::uint8_t
{
    unknown = 0,

    not_started,
    started,
    parse_method,
    parse_method_finished,
    parse_target,
    parse_target_finished,
    parse_protocol_version,
    parse_protocol_version_finished,
    finished
};

}}}

#endif //WEBCROWN_ENUMS_HPP
