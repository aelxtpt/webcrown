#ifndef ADMIN_CONFIG_HPP
#define ADMIN_CONFIG_HPP

#include <vector>

#include <webcrown/orm/databases/postgres/postgres.hpp>

using namespace webcrown;

struct User
{
    int id{};
    std::string name{};
    std::string email{};
    std::string password{};
    std::string cpf{};
};

REFL_AUTO(
    type(User, orm::Table{"Users"}), // lol, User is reserved word in postgres
    field(id, orm::Column{"id", orm::DataType::bigserial, 0, orm::ColumnFlags::primarykey | orm::ColumnFlags::ignore_insert | orm::ColumnFlags::ignore_on_serializer,}),
    field(name, orm::Column{"name", orm::DataType::text}),
    field(email, orm::Column{"email", orm::DataType::text, 0, orm::ColumnFlags::none, "UNIQUE"}),
    field(password, orm::Column{"password", orm::DataType::text, 0, orm::ColumnFlags::ignore_on_serializer}),
    field(cpf, orm::Column{"cpf", orm::DataType::text})
);

class AdminConfig
{
    std::vector<webcrown::orm::Table> tables_;
public:

    template<typename T>
    void register_model(T model);
};

static std::unique_ptr<AdminConfig> GlobalConfig = std::make_unique<AdminConfig>();

template<typename T>
void AdminConfig::register_model(T)
{
    using namespace refl;
    using Td = type_descriptor<T>;

    static_assert(descriptor::has_attribute<webcrown::orm::Table>(Td{}));


}

#endif // ADMIN_CONFIG_HPP
