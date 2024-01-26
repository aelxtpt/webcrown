#pragma once

#include <refl.hpp>
#include <vector>

struct ModelMetaData
{
    std::string verbose_name;
    std::string verbose_name_plural;
    std::string display_entry_name;
};

struct ModelDescriptionItem
{
    std::string field_name;
    std::string field_type;

    explicit ModelDescriptionItem(std::string field_name, std::string field_type)
        : field_name(std::move(field_name)), field_type(std::move(field_type))
    {}
};

REFL_AUTO(
    type(ModelDescriptionItem),
    field(field_name),
    field(field_type)
)

struct ModelDataField
{
    std::string field_name;
    std::string field_type;
    std::string field_value;
    bool is_admin_field_identifier{false};

    explicit ModelDataField(
        std::string field_name, 
        std::string field_type, 
        std::string field_value, 
        bool is_admin_field_identifier = false)
        : field_name(std::move(field_name))
        , field_type(std::move(field_type))
        , field_value(std::move(field_value))
        , is_admin_field_identifier(is_admin_field_identifier)
    {}
};

REFL_AUTO(
    type(ModelDataField),
    field(field_name),
    field(field_type),
    field(field_value),
    field(is_admin_field_identifier)
)

struct ModelData
{
    std::string model_name;
    std::vector<ModelDataField> fields;

    ModelData() = default;

    explicit ModelData(
        std::string model_name, 
        std::vector<ModelDataField> fields)
        : model_name(std::move(model_name))
        , fields(std::move(fields))
    {}
};