#pragma once

#include <string>
#include <stdexcept>
#include <cassert>
#include "webcrown/server/http/http_method.hpp"
#include <vector>

namespace webcrown {
namespace server {
namespace http {

// https://tools.ietf.org/html/rfc3986
class route 
{
    using path_parameter_name = std::string;
    using path_parameter_value = std::string;
    using path_parameters_type = std::vector<std::pair<path_parameter_name, path_parameter_value>>;

    // TODO: Parse at compile time ?
    // This handler will match /user/alex, but not will match /user/ or /user
    // /user/:username
    //
    // This handler will match /user/alex?firstname=lucas&lastname=Lima
    // /user/alex
    //
    // /user/
    //
    std::string path_;
    std::string uri_target_;
    http_method method_;
    path_parameters_type path_parameters_;
public:
    explicit route(http_method method, std::string_view path)
        : path_(path)
        , method_(method)
    {
        parse();
    }

    std::string uri_target() const noexcept { return uri_target_; }

    bool is_match_with_target_request(std::string_view target);

    path_parameters_type path_parameters() const noexcept { return path_parameters_; }

private:
    void parse();
    std::string extract_next_bind_key(std::string::const_iterator& it, std::string::const_iterator& last);

    template <typename T>
    T build_var(std::string v)
    { return (T)v; }
};

inline
bool
route::is_match_with_target_request(std::string_view target)
{
    if(!target.starts_with('/'))
    {
        // Error
        return false;
    }

    if (target.empty())
    {
        // Error
        return false;
    }

    auto begin = target.cbegin();
    auto end = target.cend();
    auto it = begin;

    // OK, we have an assert if the path doest not starts with '/',
    // so we can assume that it+1 will skip the first slash
    it++; // eat the '/'

    // Find the first slash '/'
    // It will be the real target
    for(; it < end; ++it)
    {
        if (*it == '/')
            break;
    }

    auto real_target = std::string(begin, ++it);

    if (real_target != uri_target_)
        return false;

    // Ok, the request is the our route
    // Now, we will parse the parameters

    // TODO: adjust to catch the query parameters with ?

    auto find_next_path_parameter_value = [](const char* first, const char* last) -> std::optional<std::string>
    {
        auto begin_on_current_buffer = first;
        auto& current_it = first;
        // find the end slash delimiter, that indicates end of the path parameter value
        for(; current_it < last; ++current_it)
        {
            // Have a next path parameter value ? If yes, we already have our parameter value
            if (*current_it == '/')
            {
                return std::string(begin_on_current_buffer, current_it);
            }
        }

        // Ok, we have only one parameter value
        return std::string(begin_on_current_buffer, current_it);
    };

    // Expected path parameters
    for(auto first = path_parameters_.begin(); first < path_parameters_.end(); ++first)
    {
        auto path_parameter_value = find_next_path_parameter_value(it, end);
        if (path_parameter_value)
            first->second.swap(*path_parameter_value);
    }


    return true;
}

inline
void
route::parse()
{
    // E melhor deixar para o dev converter os parametros no tipo, todos os parametros vao ser string

    if (!path_.starts_with('/'))
    {
        // TODO: MOVE TO COMPILATION TIME ERROR
        throw std::runtime_error("Route not started with slash '/'");
    }

    if (path_.empty())
    {
        // TODO: MOVE TO COMPILATION TIME ERROR
        throw std::runtime_error("Route is empty");
    }

    auto begin = path_.cbegin();
    auto end = path_.cend();
    auto it = begin;

    // OK, we have an assert if the path doest not starts with '/',
    // so we can assume that it+1 will skip the first slash
    it++; // eat the '/'

    // find last occurence of the '/'
    for(; it < end; ++it)
    {
        // Find occurence
        if (*it == '/')
        {
            // Is the last ?
            if (*(it + 1) == ':')
            {
                uri_target_ = std::string(begin, ++it);
                break;
            }
        }
    }

    if (uri_target_.empty())
    {
        // TODO: move to compile time error
        throw std::runtime_error("uri_target is empty");
    }

    // try find bind values
    for(; it < end; ++it)
    {
        if (*it == ':')
        {
            auto key = extract_next_bind_key(it, end);
            path_parameters_.push_back({key, ""});
        }
    }

}

inline
std::string
route::extract_next_bind_key(std::string::const_iterator& it, std::string::const_iterator& last)
{
    std::string result{};

    // eat the :
    auto const begin = ++it;

    for(; it < last; ++it)
    {
        // Have a next key ?
        if (*it == '/' && *(it + 1) == ':')
        {
            result = std::string(begin, it);
            return result;
        }
    }

    // Have an '/' at end ?
    if (*(last -1) == '/')
    {
        result = std::string(begin, last-1);
        return result;
    }

    // Ok, no has a next key
    result = std::string(begin, last);

    return result;
}

} // namespace http
} // namespace server
} // namespace webcrown
