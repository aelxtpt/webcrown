#pragma once

#include <webcrown/orm/databases/postgres/postgres.hpp>
#include <cstdint>
#include <string>


struct User
{
    int id{};
    std::string username{};
    std::string password{};
    std::string first_name{};
    std::string last_name{};
    std::string email{};
    bool user_active{false};
    bool user_staff_status{false};
    bool user_superuser_status{false};
    bool is_blocked{false};
    std::string blocked_reason{};
};

REFL_AUTO(
    type(User, webcrown::orm::Table{"Users"}), // lol, User is reserved word in postgres
    field(id, webcrown::orm::Column{"id", webcrown::orm::DataType::primarykey, 0, webcrown::orm::ColumnFlags::ignore_insert | webcrown::orm::ColumnFlags::ignore_on_serializer,}),
    field(username, webcrown::orm::Column{"username", webcrown::orm::DataType::text, 0, webcrown::orm::ColumnFlags::none, "UNIQUE", true}),
    field(password, webcrown::orm::Column{"password", webcrown::orm::DataType::text, 0, webcrown::orm::ColumnFlags::ignore_on_serializer}),
    field(first_name, webcrown::orm::Column{"first_name", webcrown::orm::DataType::text}),
    field(last_name, webcrown::orm::Column{"last_name", webcrown::orm::DataType::text}),
    field(email, webcrown::orm::Column{"email", webcrown::orm::DataType::text, 0, webcrown::orm::ColumnFlags::none, "UNIQUE"}),
    field(user_active, webcrown::orm::Column{"user_active", webcrown::orm::DataType::boolean}),
    field(user_staff_status, webcrown::orm::Column{"user_staff_status", webcrown::orm::DataType::boolean}),
    field(user_superuser_status, webcrown::orm::Column{"user_superuser_status", webcrown::orm::DataType::boolean}),
    field(is_blocked, webcrown::orm::Column{"is_blocked", webcrown::orm::DataType::boolean}),
    field(blocked_reason, webcrown::orm::Column{"blocked_reason", webcrown::orm::DataType::text})
);