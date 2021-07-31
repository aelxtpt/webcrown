#include "common/string/string.hpp"

namespace webcrown {
namespace common {

std::string string_utils::unescape(std::string_view s)
{
    uint16_t count{};

    for(char it : s)
    {
        count++;
    }

    return std::string("");
}

}
}