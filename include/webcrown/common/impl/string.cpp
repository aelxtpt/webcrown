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


std::vector<std::string>
string_utils::split(std::string_view str, char delimiter, bool skip_empty)
{
    std::vector<std::string> tokens;

    size_t pos_current;
    size_t pos_last = 0;
    size_t length;

    while (true)
    {
        pos_current = str.find(delimiter, pos_last);
        if (pos_current == std::string::npos)
            pos_current = str.size();

        length = pos_current - pos_last;
        if (!skip_empty || (length != 0))
            tokens.emplace_back(str.substr(pos_last, length));

        if (pos_current == str.size())
            break;
        else
            pos_last = pos_current + 1;
    }

    return tokens;
}

}
}
