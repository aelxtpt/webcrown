#include "webcrown/common/string/string.hpp"

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

bool string_utils::starts_with(std::string_view source, std::string_view prefix)
{
    return (source.size() >= prefix.size()) &&
            (source.compare(0, prefix.size(), prefix) == 0);
}

}
}
