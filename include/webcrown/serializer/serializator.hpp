#pragma once


#include <webcrown/orm/orm.hpp>
#include <exception>
#include <refl.hpp>
#include <nlohmann/json.hpp>

#include <stdlib.h>
#include <iostream>
#include <type_traits>


namespace webcrown {
namespace serializator {

using std::string;

inline
std::string normalize_type(int v)
{
    return std::to_string(v);
}

inline
std::string normalize_type(float v)
{
    std::ostringstream os;
    os << v;
    return os.str();
}

inline
std::string normalize_type(double v)
{
    std::ostringstream os;
    os << v;
    return os.str();
}

inline
std::string normalize_type(date::year_month_day v)
{
    std::ostringstream os;
    os << date::format("%Y-%m-%d", v);

    return os.str();
}

inline
std::string normalize_type(bool v)
{
    std::ostringstream os;
    os << std::boolalpha << v;

    return os.str();
}

inline
std::string normalize_type(date_time v)
{
    return v.str();
}

inline
std::string normalize_type(long v)
{
    return std::to_string(v);
}

inline
std::string normalize_type(short v)
{
    return std::to_string(v);
}

inline
string normalize_type(string v)
{
    return v;
}


struct StructSerializable : refl::attr::usage::type
{
};

struct StructMemberSerializable: refl::attr::usage::field
{};

template<typename T>
inline
nlohmann::json serialize(T obj)
{
    using json = nlohmann::json;

    using namespace refl;
    using Td = type_descriptor<T>;

    json body;

    util::for_each(
        Td::members, 
    [&body, &obj](auto member, auto index)
    {
        using MT = typename decltype(member)::value_type;

        const char* fname = refl::descriptor::get_display_name(member);

        constexpr auto column = descriptor::get_attribute<orm::Column>(member);
        if constexpr (
            (static_cast<bool>(column.attribute & orm::ColumnFlags::ignore_on_serializer))
        )
        {
            return;
        }
        
        auto v = refl::runtime::invoke<MT>(obj, fname);

        constexpr auto is_enum = std::is_enum_v<MT>;

        auto&& tv = refl::runtime::invoke<MT>(obj, fname);

        if constexpr (is_enum)
        {
            body[fname] = serializator::normalize_type(std::underlying_type_t<MT>(tv));
        }
        else if constexpr(std::is_integral_v<MT> || std::is_floating_point_v<MT>)
        {
            body[fname] = tv;
        }
        else
        {
            body[fname] = serializator::normalize_type(tv);
        }
    });

    return body;
}

template<typename T>
nlohmann::json serialize(std::vector<T> const& obj)
{
    using json = nlohmann::json;
    json result{};

    try
    {
        json data = json::array();

        for(auto const& o : obj)
        {
            auto r = serialize(o);
            data.push_back(r);
        }

        result = data;
    }
    catch(std::exception const& ex)
    {

    }

    return result;
}

template <typename C> struct is_vector : std::false_type {};    
template <typename T,typename A> struct is_vector< std::vector<T,A> > : std::true_type {};    
template <typename C> inline constexpr bool is_vector_v = is_vector<C>::value;

template<typename T>
inline
nlohmann::json serialize_admin(T obj)
{
    using json = nlohmann::json;

    using namespace refl;
    using Td = type_descriptor<T>;

    json body;

    util::for_each(
        Td::members, 
    [&body, &obj](auto member, auto index)
    {
        using MT = typename decltype(member)::value_type;

        const char* fname = refl::descriptor::get_display_name(member);

        auto v = refl::runtime::invoke<MT>(obj, fname);

        constexpr auto is_enum = std::is_enum_v<MT>;
        constexpr auto is_vector = is_vector_v<MT>;

        if constexpr (is_enum)
        {
            body[fname] = serializator::normalize_type(std::underlying_type_t<MT>(v));
        }
        else if constexpr(is_vector)
        {
            body[fname] = serialize_admin(v);
        }
        else if constexpr(std::is_integral_v<MT> || std::is_floating_point_v<MT>)
        {
            body[fname] = v;
        }
        else
        {
            body[fname] = serializator::normalize_type(v);
        }
    });

    return body;
}

template<typename T>
nlohmann::json serialize_admin(std::vector<T> const& obj)
{
    using json = nlohmann::json;
    json result{};

    try
    {
        json data = json::array();

        for(auto const& o : obj)
        {
            auto r = serialize_admin(o);
            data.push_back(r);
        }

        result = data;
    }
    catch(std::exception const& ex)
    {

    }

    return result;
}

}} // namespace webcrown::serializator