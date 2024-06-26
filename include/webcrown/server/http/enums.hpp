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
    parse_start_line_started,
    parse_start_line,
    parse_method,
    parse_method_finished,
    parse_target,
    parse_target_finished,
    parse_protocol_version,
    parse_protocol_version_finished,
    parse_headers,
    parse_headers_finished,
    parse_content_type,
    parse_content_type_finished,
    parse_media_type,
    parse_media_type_need_more,
    parse_media_type_finished,
    parse_body,
    parse_body_finished,
    parse_body_pending,
    finished
};

}}}

#endif //WEBCROWN_ENUMS_HPP
