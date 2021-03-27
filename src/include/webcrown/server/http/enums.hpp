#ifndef WEBCROWN_ENUMS_HPP
#define WEBCROWN_ENUMS_HPP

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
    parse_target,
    parse_protocol_version,
    finished
};

}}}

#endif //WEBCROWN_ENUMS_HPP
