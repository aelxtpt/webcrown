#pragma once

#include <string>
#include <vector>

namespace webcrown {
namespace common {

class string_utils
{
public:
    static std::string unescape(std::string_view s);

    static bool starts_with(std::string_view source, std::string_view prefix);

    static std::vector<std::string> split(std::string_view str, char delimiter, bool skip_empty = false);

    static char to_lower(char ch);

    static char to_upper(char ch);

    static std::string to_lower(std::string_view str);

    static std::string to_upper(std::string_view str);

    static std::string& lower(std::string& str);
 
    static std::string& upper(std::string& str);

    static bool replace_all(std::string& str, std::string_view substr, std::string_view with);


private:
    static char to_lower_internal(char ch);
    static char to_upper_internal(char ch);
};

}}
