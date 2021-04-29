#ifndef WEBCROWN_METHOD_INL
#define WEBCROWN_METHOD_INL

namespace webcrown {
namespace server {
namespace http {

std::string_view
to_string(http_method m)
{
    return "";
}

http_method
to_method(std::string_view m)
{
    // minimum http get_method size
    if (m.size() < 3)
        return http_method::unknown;

    if (m == "GET")
        return http_method::get;
    else if(m == "POST")
        return http_method::post;
    else if(m == "DELETE")
        return http_method::delet;
    else
        return http_method::unknown;
}

}}}

#endif //WEBCROWN_METHOD_INL
