#pragma once

#include <string>

namespace webcrown {
namespace common {

class string_utils
{
public:
    static std::string unescape(std::string_view s);
};

}}