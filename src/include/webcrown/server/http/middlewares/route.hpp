#pragma once

namespace webcrown {
namespace server {
namespace http {

#include <string>

class route
{
    // Parse at compile time ?
    //  path('articles/2003/'
    //    path('articles/<int:year>/'
    //    path('articles/<int:year>/<int:month>/'
    //    path('articles/<int:year>/<int:month>/<slug:slug>/'
    std::string path_;
    std::string uri_target_;
public:
    explicit route(std::string_view path)
        : path_(path)
    {}

private:
    void parse();
};

}}}