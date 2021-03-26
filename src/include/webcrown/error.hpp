#ifndef WEBCROWN_ERROR_HPP
#define WEBCROWN_ERROR_HPP

#include <system_error>

namespace webcrown {

class webcrown_error : public std::error_category
{
public:
    const char* name() const noexcept override
    {
        return "webcrown error";
    }

    std::string message(int ec) const override
    {
        return "";
    }

};

}

#endif //WEBCROWN_ERROR_HPP
