#ifndef WEBCROWN_METHOD_INL
#define WEBCROWN_METHOD_INL

namespace webcrown {
namespace server {
namespace http {

std::string_view
to_string(method m)
{
    return "";
}

method
to_method(std::string_view m)
{
    // minimum http method size
    if (m.size() < 3)
        return method::unknown;

    if (m == "GET")
        return method::get;
    else if(m == "POST")
        return method::post;
    else if(m == "DELETE")
        return method::delet;
    else
        return method::unknown;
}

}}}

#endif //WEBCROWN_METHOD_INL
