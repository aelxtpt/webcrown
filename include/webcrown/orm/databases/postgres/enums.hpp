#pragma once

#include <cstdint>
#include <type_traits>

namespace webcrown {
namespace orm {

enum class DataType
{
    primarykey, // bigserial with primary key
    bigint, // signed eight-byte integer
    bigserial, // autoincrementing eight-byte integer
    boolean, // logical boolean (true/false)
    char_n, // fixed-length character string char[n]
    cidr, // ipv4 or ipv6 network address
    date, // calendar date (year, month, day)
    integer,
    decimal, // exact numeric of selectable precision
    smallint, // int2, signed two-byte integer
    smallserial, // serial2, autoincrementing two-byte integer
    serial, // serial4, autoincremeting four-byte integer
    text, // variable length character string
    timestamp, // date and time (no time zone),
    time, // time of day (no date)
};

enum class ColumnFlags : uint8_t
{
    none = 0b00000000,
    primarykey = 0b00000001,
    ignore_insert = 0b00000010,
    ignore_on_serializer = 0b00000100
};

constexpr ColumnFlags operator|(ColumnFlags lhs, ColumnFlags rhs)
{
    return static_cast<ColumnFlags>(
        static_cast<std::underlying_type_t<ColumnFlags>>(lhs) |
        static_cast<std::underlying_type_t<ColumnFlags>>(rhs)
    );
};

// constexpr ColumnFlags operator&(ColumnFlags lhs, ColumnFlags rhs)
// {
//     return static_cast<ColumnFlags>(
//         static_cast<std::underlying_type_t<ColumnFlags>>(lhs) &
//         static_cast<std::underlying_type_t<ColumnFlags>>(rhs)
//     );
// };

// TODO: Fix constness
constexpr ColumnFlags operator&(const ColumnFlags lhs, ColumnFlags rhs)
{
    return static_cast<ColumnFlags>(
        static_cast<std::underlying_type_t<ColumnFlags>>(lhs) &
        static_cast<std::underlying_type_t<ColumnFlags>>(rhs)
    );
}

}}