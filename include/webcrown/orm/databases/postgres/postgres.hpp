#pragma once


#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ios>
#include <limits>
#include <numeric>
#include <optional>
#include <sstream>
#include <stdio.h>
#include <string>
#include <algorithm>
#include <iostream>
#include <sys/types.h>
#include <tuple>
#include <type_traits>
#include <charconv>
#include <ostream>

#include <refl.hpp>
#include <pqxx/pqxx>
#include <date/date.h>
#include <date/tz.h>
#include "enums.hpp"

#include "webcrown/common/date/date_time.hpp"
#include "webcrown/common/meta/to_string.hpp"

namespace webcrown {
namespace orm {

using std::string;
using std::vector;

using ConnectionT = pqxx::connection;
using day_point = 
    std::chrono::time_point<std::chrono::system_clock, date::days>;

struct Table : refl::attr::usage::type
{
    const char* name{};
    const char* constraints{};

    constexpr Table(const char* name, const char* constraints = "") noexcept
        : name(name)
        , constraints(constraints)
    {}
};

struct Column : refl::attr::usage::field
{
    const char* name;
    const DataType data_type;
    const ColumnFlags attribute;
    const char* custom_definition_flags;
    const int32_t length;

    constexpr Column(const char* name, DataType dataType, uint32_t length = -1, ColumnFlags attribute = ColumnFlags::none, const char* custom_definition_flags = "") noexcept
        : name(name)
        , data_type(dataType)
        , attribute(attribute)
        , custom_definition_flags(custom_definition_flags)
        , length(length)
    {}
};


namespace query {

template<typename Member>
constexpr auto make_sql_field_spec(Member)
{
    using namespace refl;
    using MT = typename Member::value_type;

    // Put the traits here, can generate compile error, because some types can conflict. Example integer trait with char[2]. numeric_limits conflict

    constexpr auto column = descriptor::get_attribute<Column>(Member{});
    
    // REFL_MAKE_CONST_STRING convert the const char* data member to a refl::const_string<N>
    // (necessary to be able to do compile-time string concat)

    constexpr bool should_apply_custom_define = ColumnFlags::none == column.attribute && REFL_MAKE_CONST_STRING(column.custom_definition_flags) != "";

    if constexpr (DataType::primarykey == column.data_type)
    {
        constexpr bool is_integral = std::is_integral_v<MT>;
        static_assert(is_integral && "Type should be an integral type like int");

        return REFL_MAKE_CONST_STRING(column.name) + " bigserial PRIMARY KEY";
    }
    else if constexpr (DataType::bigserial == column.data_type)
    {
        constexpr bool is_integral = std::is_integral_v<MT>;
        static_assert(is_integral && "Type should be an integral type like int");

        if constexpr (should_apply_custom_define)
            return REFL_MAKE_CONST_STRING(column.name) + " bigserial " + REFL_MAKE_CONST_STRING(column.custom_definition_flags);
        else
            return REFL_MAKE_CONST_STRING(column.name) + " bigserial";
    }
    else if constexpr (DataType::text == column.data_type)
    {
        constexpr bool is_text = std::is_same_v<MT, std::string> || std::is_same_v<MT, const char*>;
        static_assert(is_text && "Type should be a text type like std::string, const char*");

        if constexpr (should_apply_custom_define)
            return REFL_MAKE_CONST_STRING(column.name) + " TEXT " + REFL_MAKE_CONST_STRING(column.custom_definition_flags);
        else
            return REFL_MAKE_CONST_STRING(column.name) + " TEXT";
    }
    else if constexpr (DataType::integer == column.data_type)
    {
        constexpr bool is_integer = 
            std::numeric_limits<MT>::is_signed && std::numeric_limits<MT>::is_integer ||
            std::is_enum_v<MT>;
        static_assert(is_integer && "Type should be a integer like int");

        if constexpr (should_apply_custom_define)
            return REFL_MAKE_CONST_STRING(column.name) + " integer " + REFL_MAKE_CONST_STRING(column.custom_definition_flags);
        else
            return REFL_MAKE_CONST_STRING(column.name) + " integer";
    }
    else if constexpr (DataType::bigint == column.data_type)
    {
        constexpr bool is_bigint = std::is_same_v<MT, int64_t>;
        static_assert(is_bigint && "Type should be a bigint like int64_t, long");

        if constexpr (should_apply_custom_define)
            return REFL_MAKE_CONST_STRING(column.name) + " bigint " + REFL_MAKE_CONST_STRING(column.custom_definition_flags);
        else
            return REFL_MAKE_CONST_STRING(column.name) + " bigint";
    }
    else if constexpr (DataType::boolean == column.data_type)
    {
        constexpr bool is_bool = std::is_same_v<MT, bool>;
        static_assert(is_bool && "Type should be boolean");

        if constexpr (should_apply_custom_define)
            return REFL_MAKE_CONST_STRING(column.name) + " boolean " + REFL_MAKE_CONST_STRING(column.custom_definition_flags);
        else
            return REFL_MAKE_CONST_STRING(column.name) + " boolean";
    }
    else if constexpr (DataType::char_n == column.data_type)
    {
        static_assert(column.length != -1 && "The length should be different than -1");

        constexpr auto charlength = "char[" + REFL_MAKE_CONST_STRING(meta::to_string<column.length>) + "]";

        if constexpr (should_apply_custom_define)
            return REFL_MAKE_CONST_STRING(column.name) + " " + (charlength) + " " + REFL_MAKE_CONST_STRING(column.custom_definition_flags);
        else
            return REFL_MAKE_CONST_STRING(column.name) + " " + (charlength);
    }
    else if constexpr (DataType::cidr == column.data_type)
    {
        constexpr bool is_text = std::is_same_v<MT, std::string> || std::is_same_v<MT, const char*>;
        static_assert(is_text && "Type should be a text type like std::string, const char*");

        if constexpr (should_apply_custom_define)
            return REFL_MAKE_CONST_STRING(column.name) + " cidr " + REFL_MAKE_CONST_STRING(column.custom_definition_flags);
        else
            return REFL_MAKE_CONST_STRING(column.name) + " cidr";
    }
    else if constexpr (DataType::date == column.data_type)
    {
        constexpr auto is_date = std::is_same_v<MT, date::year_month_day>;
        static_assert(is_date && "Type should be a date");

        if constexpr (should_apply_custom_define)
            return REFL_MAKE_CONST_STRING(column.name) + " date " + REFL_MAKE_CONST_STRING(column.custom_definition_flags);
        else
            return REFL_MAKE_CONST_STRING(column.name) + " date";
    }
    else if constexpr (DataType::decimal == column.data_type)
    {
        constexpr auto is_decimal = std::is_floating_point_v<MT>;
        static_assert(is_decimal && "Type should be a floating pointer");

        if constexpr (should_apply_custom_define)
            return REFL_MAKE_CONST_STRING(column.name) + " decimal " + REFL_MAKE_CONST_STRING(column.custom_definition_flags);
        else
            return REFL_MAKE_CONST_STRING(column.name) + " decimal";
    }
    else if constexpr (DataType::smallint == column.data_type)
    {
        constexpr auto is_smallint = std::is_same_v<MT, int16_t> || std::is_same_v<MT, short>;
        static_assert(is_smallint && "Type should be smallint");

        if constexpr (should_apply_custom_define)
            return REFL_MAKE_CONST_STRING(column.name) + " smallint " + REFL_MAKE_CONST_STRING(column.custom_definition_flags);
        else
            return REFL_MAKE_CONST_STRING(column.name) + " smallint";
    }
    else if constexpr (DataType::smallserial == column.data_type)
    {
        constexpr auto is_smallint = std::is_same_v<MT, int16_t> || std::is_same_v<MT, short>;
        static_assert(is_smallint && "Type should be smallint");

        if constexpr (should_apply_custom_define)
            return REFL_MAKE_CONST_STRING(column.name) + " smallserial " + REFL_MAKE_CONST_STRING(column.custom_definition_flags);
        else
            return REFL_MAKE_CONST_STRING(column.name) + " smallserial";
    }
    else if constexpr (DataType::serial == column.data_type)
    {
        constexpr auto is_serial = std::is_same_v<MT, int32_t>;
        static_assert(is_serial && "Type should be int 4 bytes");

        if constexpr (should_apply_custom_define)
            return REFL_MAKE_CONST_STRING(column.name) + " serial " + REFL_MAKE_CONST_STRING(column.custom_definition_flags);
        else
            return REFL_MAKE_CONST_STRING(column.name) + " serial";
    }
    else if constexpr (DataType::timestamp == column.data_type)
    {
        constexpr auto is_timestamp = std::is_same_v<MT, date_time>;
        static_assert (is_timestamp && "Type should be date_time");

        if constexpr (should_apply_custom_define)
            return REFL_MAKE_CONST_STRING(column.name) + " timestamp " + REFL_MAKE_CONST_STRING(column.custom_definition_flags);
        else
            return REFL_MAKE_CONST_STRING(column.name) + " timestamp";
    }
    else if constexpr (DataType::time == column.data_type)
    {
        constexpr auto is_time = std::is_same_v<MT, date::time_of_day<std::chrono::seconds>>;
        static_assert (is_time && "Type should be date::time_of_day<std::chrono::seconds>");

        if constexpr (should_apply_custom_define)
            return REFL_MAKE_CONST_STRING(column.name) + " time " + REFL_MAKE_CONST_STRING(column.custom_definition_flags);
        else
            return REFL_MAKE_CONST_STRING(column.name) + " time";
    }
}

// if the type has a namespace, it will throw compile error
template<typename T>
constexpr auto create_table()
{
    using namespace refl;
    using Td = type_descriptor<T>;

    static_assert(descriptor::has_attribute<Table>(Td{}));

    // Concatenate all the members column definitions together
    constexpr auto fields = util::accumulate(
        Td::members,
        [](auto acc, auto member)
        {
            return acc + ",\n\t" + make_sql_field_spec(member);
        },
        make_const_string())
        .template substr<2>(); // reomve the initial ",\n"

    constexpr auto tbl = descriptor::get_attribute<Table>(Td{});
    // convert the const char* data member to a refl::const_string<N>
    // (necessary to be able to do compile-time string concat)

    constexpr auto tbl_name = REFL_MAKE_CONST_STRING(tbl.name);
    constexpr auto tbl_constraints = REFL_MAKE_CONST_STRING(tbl.constraints);

    if constexpr (util::detail::strlen(tbl.constraints) > 0)
        return "CREATE TABLE IF NOT EXISTS " + tbl_name + " (\n" + fields + ",\n" + tbl_constraints + "\n);";
    else
        return "CREATE TABLE IF NOT EXISTS " + tbl_name + " (\n" + fields + "\n);";
};



template<typename T, bool GenPlaceHolders = true>
constexpr auto insert()
{
    using namespace refl;
    using Td = type_descriptor<T>;

    static_assert(descriptor::has_attribute<Table>(Td{}));

    constexpr auto fields_to_insert = util::accumulate(
        Td::members,
        [](auto acc, auto member)
        {
            constexpr auto column = descriptor::get_attribute<Column>(member);
            if constexpr (
                (static_cast<bool>(column.attribute & ColumnFlags::ignore_insert)) ||
                (static_cast<bool>(column.attribute & ColumnFlags::primarykey) ||
                (column.data_type == DataType::primarykey))
            )
            {
                return acc;
            }

            return acc + 1;
        }, 0);

    static_assert(fields_to_insert >= 1 && "Insert should have at least 1 field without ignore_insert or primary key");

    constexpr auto fields = util::accumulate(
        Td::members,
        [](auto acc, auto member)
        {
            // acc is accumulated string
            constexpr auto column = descriptor::get_attribute<Column>(member);

            if constexpr (static_cast<bool>(column.attribute & ColumnFlags::ignore_insert))
                return acc;
            else if constexpr (ColumnFlags::primarykey == column.attribute)
                return acc;
            else
                return acc + ",\n\t " + REFL_MAKE_CONST_STRING(column.name);
        },
        make_const_string())
        .template substr<2>();  // remove the initial ",\n";
    
    constexpr auto tbl = descriptor::get_attribute<Table>(Td{});
    constexpr auto tbl_name = REFL_MAKE_CONST_STRING(tbl.name);

    if constexpr (GenPlaceHolders)
    {
        constexpr auto values = util::accumulate(
            Td::members,
            [](auto acc, auto member)
            {
                constexpr auto column = descriptor::get_attribute<Column>(member);

                // It does not working as expected, if remove if GenPlaceHolders, it will throw error
                if constexpr (static_cast<bool>(column.attribute & ColumnFlags::ignore_insert))
                    return acc;

                constexpr auto index = meta::get_index_member(member, Td::members);

                if constexpr (ColumnFlags::primarykey == column.attribute)
                    return acc;
                else
                    return acc + ",\n\t $" + REFL_MAKE_CONST_STRING(meta::to_string<index>);
            },
            make_const_string())
            .template substr<2>();
    
        return "INSERT INTO " + tbl_name + " (\n" + fields + "\n) VALUES" + " (\n" + values + "\n)";
    }
    else
        return "INSERT INTO " + tbl_name + " (\n" + fields + "\n) ";
};

template<typename T>
constexpr auto select()
{
    using namespace refl;
    using Td = type_descriptor<T>;

    static_assert(descriptor::has_attribute<Table>(Td{}));
    
    constexpr auto tbl = descriptor::get_attribute<Table>(Td{});
    constexpr auto tbl_name = REFL_MAKE_CONST_STRING(tbl.name);

    return "SELECT * FROM " + tbl_name + " ";
};

template<typename T>
constexpr auto drop_table()
{
    using namespace refl;
    using Td = type_descriptor<T>;

    static_assert(descriptor::has_attribute<Table>(Td{}));

    constexpr auto tbl = descriptor::get_attribute<Table>(Td{});
    constexpr auto tbl_name = REFL_MAKE_CONST_STRING(tbl.name);

    return "DROP TABLE " + tbl_name;
};

template<typename T>
constexpr auto truncate()
{
    using namespace refl;
    using Td = type_descriptor<T>;

    static_assert(descriptor::has_attribute<Table>(Td{}));

    constexpr auto tbl = descriptor::get_attribute<Table>(Td{});
    constexpr auto tbl_name = REFL_MAKE_CONST_STRING(tbl.name);

    return "TRUNCATE TABLE " + tbl_name;
};

template<typename T>
constexpr auto delete_()
{
    using namespace refl;
    using Td = type_descriptor<T>;

    static_assert(descriptor::has_attribute<Table>(Td{}));
    
    constexpr auto tbl = descriptor::get_attribute<Table>(Td{});
    constexpr auto tbl_name = REFL_MAKE_CONST_STRING(tbl.name);

    return "DELETE FROM " + tbl_name + " ";
};

template<typename T>
constexpr auto update()
{
    using namespace refl;
    using Td = type_descriptor<T>;

    static_assert(descriptor::has_attribute<Table>(Td{}));
    
    constexpr auto tbl = descriptor::get_attribute<Table>(Td{});
    constexpr auto tbl_name = REFL_MAKE_CONST_STRING(tbl.name);

    return "UPDATE " + tbl_name + " SET ";
};

} // namespace query


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
    os << "'" << date::format("%Y-%m-%d", v) << "'";

    return os.str();
}

inline
std::string normalize_type(date::time_of_day<std::chrono::seconds> v)
{
    std::ostringstream os;
    os << v;

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
    return "'" + v.str() + "'";
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
    return "'" + v + "'";
}

static std::vector<std::string> split(std::string_view str, char delimiter, bool skip_empty = false)
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

template<typename T>
bool create_table(ConnectionT& c)
{
    pqxx::work w(c);

    constexpr auto create_sql = query::create_table<T>();

    w.exec0(create_sql.str());
    w.commit();
    
    return true;
};

template<typename T>
bool insert(T& v, ConnectionT& c)
{
    using std::string;
    using namespace refl;
    using Td = type_descriptor<T>;

    static_assert(descriptor::has_attribute<Table>(Td{}));

    auto front_query_sql = query::insert<T, false>().str();
    front_query_sql += "VALUES (";
    string query_sql = "";

    util::for_each(Td::members, 
    [&v, &query_sql](auto member, auto index)
    {
        constexpr auto column = descriptor::get_attribute<Column>(member);
        using MT = typename decltype(member)::value_type;

        if constexpr (
            static_cast<bool>(column.attribute & ColumnFlags::ignore_insert) ||
            static_cast<bool>(column.attribute & ColumnFlags::primarykey))
            return;
            
        // TODO: Fix char type, segment fault  lol
        //std::cout << refl::runtime::invoke<MT>(v, refl::descriptor::get_display_name(member)) << "\n";
        
        auto&& tv = refl::runtime::invoke<MT>(
                        v,
                        refl::descriptor::get_display_name(member));

        constexpr auto is_enum = std::is_enum_v<MT>;

        if constexpr (is_enum)
        {
            query_sql += 
            ",\n\t" +
                normalize_type(std::underlying_type_t<MT>(tv));
        }
        else
        {
            query_sql += 
            ",\n\t" +
                normalize_type(tv);
        }
        
    });

    query_sql = query_sql.substr(1);
    query_sql += "\n);";

    auto final_query = front_query_sql + query_sql;

    std::cout << final_query << "\n";

    pqxx::work w(c);
    pqxx::result result = w.exec0(final_query);
    w.commit();

    if(result.affected_rows())
        return true;

    return false;
}

template<typename T>
std::optional<T> 
select(std::string const& cond, ConnectionT& c)
{
    using namespace refl;
    using Td = type_descriptor<T>;

    static_assert(descriptor::has_attribute<Table>(Td{}));

    std::string sql = query::select<T>().str();
    sql += cond;

    // TODO: segment fault too
    //std::cout << sql << "\n";

    pqxx::work w(c);
    pqxx::result res = w.exec(sql);

    if(res.empty())
        return std::nullopt;

    auto&& row = res.front();

    T final{};

    util::for_each(Td::members, 
    [&final, &row](auto member, auto index)
    {
        constexpr auto column = descriptor::get_attribute<Column>(member);
        using MT = typename decltype(member)::value_type;

        const char* fname = refl::descriptor::get_display_name(member);

        if constexpr (std::is_same_v<MT, date_time>)
        {
            auto v = row[fname].as<string>();
            auto x = date_time::from_str(v);

            std::cout << "V " << x << "\n";
            refl::runtime::invoke<MT>(final, fname, x);
        }
        else if constexpr (std::is_same_v<MT, date::time_of_day<std::chrono::seconds>>)
        {
            using tod = date::time_of_day<std::chrono::seconds>;
            auto v = row[fname].as<string>();

            std::istringstream ss{v};
            std::chrono::seconds d;
            ss >> date::parse("%T", d);

            tod td{d};
            refl::runtime::invoke<MT>(final, fname, td);
        }
        else if constexpr (std::is_same_v<MT, date::year_month_day>)
        {
            auto v = row[fname].as<string>();
            date::year_month_day tp;

            std::istringstream is{v};
            is >> date::parse("%Y-%m-%d", tp);

            refl::runtime::invoke<MT>(final, fname, tp);
        }
        else if constexpr (std::is_enum_v<MT>)
        {
            auto v = row[fname].as<int>();
            MT t = static_cast<MT>(v);

            refl::runtime::invoke<MT>(final, fname, t);
        }
        else
        {
            auto v = row[fname].as<MT>();
            refl::runtime::invoke<MT>(final, fname, v);
        }
        
    });

    return final;
}

template<typename T>
vector<T> select_many(string const& cond, ConnectionT& c)
{
    using namespace refl;
    using Td = type_descriptor<T>;

    static_assert(descriptor::has_attribute<Table>(Td{}));

    std::string sql = query::select<T>().str();
    sql += cond;

    std::cout << sql << "\n";

    pqxx::work w(c);

    pqxx::result res = w.exec(sql);

    if(res.empty())
        return vector<T>{};

    // TODO: Please optmize me
    // And with the PK is not good. Because a table can not have a pk

    vector<T> final{};

    for(auto const& row : res)
    {
        T obj;
           util::for_each(
        Td::members, 
        [&obj, &row](auto member, auto index)
        {
            constexpr auto column = descriptor::get_attribute<Column>(member);
            using MT = typename decltype(member)::value_type;

            const char* fname = refl::descriptor::get_display_name(member);

            if constexpr (std::is_same_v<MT, date_time>)
            {
                auto v = row[fname].as<string>();
                auto x = date_time::from_str(v);

                std::cout << "V " << x << "\n";
                refl::runtime::invoke<MT>(obj, fname, x);
            }
            else if constexpr (std::is_same_v<MT, date::time_of_day<std::chrono::seconds>>)
            {
                using tod = date::time_of_day<std::chrono::seconds>;
                auto v = row[fname].as<string>();

                std::istringstream ss{v};
                std::chrono::seconds d;
                ss >> date::parse("%T", d);

                tod td{d};
                refl::runtime::invoke<MT>(obj, fname, td);
            }
            else if constexpr (std::is_same_v<MT, date::year_month_day>)
            {
                auto v = row[fname].as<string>();
                date::year_month_day tp;

                std::istringstream is{v};
                is >> date::parse("%Y-%m-%d", tp);

                refl::runtime::invoke<MT>(obj, fname, tp);
            }
            else if constexpr (std::is_enum_v<MT>)
            {
                auto v = row[fname].as<int>();
                MT t = static_cast<MT>(v);

                refl::runtime::invoke<MT>(obj, fname, t);
            }
            else
            {
                auto v = row[fname].as<MT>();
                refl::runtime::invoke<MT>(obj, fname, v);
            }
        });

        final.push_back(std::move(obj));

        // for each row
        //std::cout << "end select\n";
    }

    return final;
}

template<typename T>
bool delete_(string const& cond, ConnectionT& c)
{
    using namespace refl;
    using Td = type_descriptor<T>;

    static_assert(descriptor::has_attribute<Table>(Td{}));

    std::string sql = query::delete_<T>().str();
    sql += cond;

    std::cout << sql << "\n";

    pqxx::work w(c);
    pqxx::result res = w.exec0(sql);
    w.commit();

    return true;
}

template<
    std::size_t I = 0,
    typename ...Args,
    typename Func>
constexpr void my_for_each(const std::tuple<Args...> &t, Func &&func)
{
    if constexpr (I < sizeof...(Args))
    {
        func(std::get<I>(t));
        my_for_each<I + 1>(t, std::forward<Func>(func));
    }
}

template<
    std::size_t I = 0,
    typename ...Args>
void up_find_value(const std::tuple<Args...> &t, const char* field_name, string& result_value)
{
    if constexpr (I < sizeof...(Args))
    {
        auto parameter =std::get<I>(t);
        auto fname = std::get<0>(parameter);
        
        if (std::strcmp(fname, field_name) == 0)
        {
            auto v = std::get<1>(parameter);
            auto nv = normalize_type(v);

            result_value = nv;
        }

        up_find_value<I + 1>(t, field_name, result_value);
    }
}

template<typename T, typename... Args>
bool update(string const& cond, ConnectionT& c, Args &&... args)
{
    using namespace refl;
    using Td = type_descriptor<T>;

    static_assert(descriptor::has_attribute<Table>(Td{}));

    std::tuple t(std::forward<Args>(args)...);

    constexpr auto field_exists = 
        [](const char* field_name) -> bool
        {
            // O(N) for each field
            auto field_count = util::accumulate(
                Td::members,
                [&field_name](auto acc, auto member)
                {
                    if (std::strcmp(refl::descriptor::get_display_name(member), field_name) == 0)
                        return acc + 1;
                    return acc;
                }, 0);

            if (field_count > 0)
                return true;

            return false;
        };

    my_for_each(t, 
        [&field_exists](auto &arg)
        {
            auto field_name = std::get<0>(arg);

            auto exists = field_exists(field_name);

            assert(exists && "Field does not exists");
        }
    );

    string update_sql = query::update<T>().str();
    
    util::for_each(
        Td::members,
        [&t, &update_sql](auto member, auto index)
        {
            constexpr auto column = descriptor::get_attribute<Column>(member);
            const char* field_name = refl::descriptor::get_display_name(member);

            string res{};
            up_find_value(t, field_name, res);

            if(!res.empty())
            {
                update_sql += "\n\t" + 
                    REFL_MAKE_CONST_STRING(column.name).str() +
                    "=" +
                    res +
                    ",";
            }
        });
    
    update_sql = update_sql.substr(0, update_sql.size() -1);
    update_sql += " " + cond;

    std::cout << update_sql << "\n";

    pqxx::work w(c);

    pqxx::result res = w.exec0(update_sql);
    w.commit();
    
    return true;
}

}}
