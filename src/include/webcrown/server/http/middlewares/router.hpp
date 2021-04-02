#pragma once

namespace webcrown {
namespace server {
namespace http {

class router
{
    //  path('articles/2003/'
    //    path('articles/<int:year>/'
    //    path('articles/<int:year>/<int:month>/'
    //    path('articles/<int:year>/<int:month>/<slug:slug>/'
    std::string path_;
public:
    explicit router(std::string_view path)
    {}
};

}}}