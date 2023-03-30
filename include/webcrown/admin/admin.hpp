#pragma once

#include <refl.hpp>
#include <bits/utility.h>
#include <boost/algorithm/string.hpp>
#include <webcrown/common/url_parser.hpp>

#include <cstddef>
#include <iterator>
#include <memory>
#include <any>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <tuple>
#include <vector>
#include <utility>
#include <type_traits>

#include <boost/lexical_cast.hpp>

#include <fmt/format.h>
#include "database/connection.hpp"
#include "webcrown/admin/user_model.hpp"
#include "refl.hpp"

#include "webcrown/admin/adm_structs.hpp"
#include "webcrown/security/password_hashing.hpp"
#include "webcrown/serializer/serializator.hpp"
#include "webcrown/orm/databases/postgres/enums.hpp"
#include "webcrown/orm/databases/postgres/postgres.hpp"
#include "webcrown/orm/orm.hpp"
#include "webcrown/server/http/middlewares/auth_middleware.hpp"
#include "webcrown/server/http/middlewares/route.hpp"
#include "webcrown/helpers/endpoint_context.hpp"

#include "inja/inja.hpp"

namespace webcrown {
namespace admin {
    

using namespace serializator;

using namespace webcrown::helpers;
using namespace refl;
using std::shared_ptr;
using std::make_shared;


template<typename T>
std::string
get_type_name_from_model(T member)
{
    using webcrown::orm::DataType;
    using namespace refl;
    using MT = typename decltype(member)::value_type;

    constexpr auto column = descriptor::get_attribute<orm::Column>(decltype(member){});

    if constexpr (
        DataType::bigserial == column.data_type ||
        DataType::integer == column.data_type ||
        DataType::bigint == column.data_type ||
        DataType::smallint == column.data_type ||
        DataType::smallserial == column.data_type ||
        DataType::serial == column.data_type)
    {
        constexpr bool is_integral = std::is_integral_v<MT>;
        static_assert(is_integral && "Type should be an integral type like int");

        return "integer";
    }
    else if constexpr(DataType::primarykey == column.data_type)
    {
        return "primary_key";
    }
    else if constexpr (DataType::text == column.data_type)
    {
        constexpr bool is_text = std::is_same_v<MT, std::string> || std::is_same_v<MT, const char*>;
        static_assert(is_text && "Type should be a text type like std::string, const char*");

        return "text";
    }
    else if constexpr (DataType::boolean == column.data_type)
    {
        constexpr bool is_bool = std::is_same_v<MT, bool>;
        static_assert(is_bool && "Type should be boolean");

        return "boolean";
    }
    else if constexpr (DataType::date == column.data_type)
    {
        constexpr auto is_date = std::is_same_v<MT, date::year_month_day>;
        static_assert(is_date && "Type should be a date");

        return "date";
    }
    else if constexpr (DataType::decimal == column.data_type)
    {
        constexpr auto is_decimal = std::is_floating_point_v<MT>;
        static_assert(is_decimal && "Type should be a floating pointer");

        return "decimal";
    }
    else if constexpr (DataType::timestamp == column.data_type)
    {
        constexpr auto is_timestamp = std::is_same_v<MT, date_time>;
        static_assert (is_timestamp && "Type should be date_time");

        return "timestamp";
    }
    else if constexpr (DataType::time == column.data_type)
    {
        constexpr auto is_time = std::is_same_v<MT, date::time_of_day<std::chrono::seconds>>;
        static_assert (is_time && "Type should be date::time_of_day<std::chrono::seconds>");

        return "time";
    }

    return "";
}


using std::vector;
using std::shared_ptr;
using std::make_shared;

using nlohmann::json;
using webcrown::server::http::route;
using webcrown::server::http::path_parameters_type;
using webcrown::server::http::http_method;
using webcrown::server::http::http_request;
using webcrown::server::http::http_context;
using webcrown::server::http::http_response;
using webcrown::server::http::http_status;
using webcrown::server::http::route_parameters_t;
using webcrown::server::http::auth_authorization_level;

template<typename... Args>
class AdminManager
{
    std::tuple<User, Args...> t;
    std::vector<std::shared_ptr<route>> routes_{};
public:

    decltype(routes_) routes() const { return routes_; }

    void register_models(Args &&... args)
    {
        t = std::tuple<User, Args...>();

        routes_.push_back(
            make_shared<route>(http_method::get, "/admin", 
            [&](http_request const& request, http_response& response, path_parameters_type const& parameters, http_context const& context)
        {
            // Just for convenience
            using namespace inja;
            using std::make_pair;
            namespace fs = std::filesystem;

            std::string html;

            try
            {
                auto names = this->models_names();

                response.add_header("content-type", "text/html; charset=UTF-8");

                auto current_path = fs::current_path().string();

                std::cout << "Current paht is " << current_path << "\n";

                Environment env;
                Template sidebar = env.parse_template("static/templates/admin/sidebar.html");
                env.include_template("sidebar", sidebar);
                Template temp = env.parse_template("static/templates/admin/index.html");

                json data;
                data["models"] = names;

                html = env.render(temp, data);

                std::cout << html << "\n";
            }
            catch(std::exception const& ex)
            {
                std::cout << ex.what() << "\n";
            }

            response.set_body(html);
            response.set_status(http_status::ok);
        }
        ));

        routes_.push_back(make_shared<route>(http_method::get, "/admin/users",
        [&](http_request const& request, http_response& response, path_parameters_type const& parameters, http_context const& context)
        {
            // Just for convenience
            using namespace inja;
            using std::make_pair;
            namespace fs = std::filesystem;

            std::string html;

            try
            {
                response.add_header("content-type", "text/html; charset=UTF-8");

                auto current_path = fs::current_path().string();

                std::cout << "Current paht is " << current_path << "\n";

                Environment env;
                Template sidebar = env.parse_template("static/templates/admin/sidebar.html");
                env.include_template("sidebar", sidebar);
                Template temp = env.parse_template("static/templates/admin/users_list.html");

                auto users_data = this->get_simple_serialized("Users");

                std::cout << users_data.dump() << "\n";

                json data;
                data["users"] = users_data;

                html = env.render(temp, data);

            }
            catch(std::exception const& ex)
            {
                std::cout << ex.what() << "\n";
            }

            response.set_body(html);
            response.set_status(http_status::ok);
        }));

        auto admin_index_users_add = 
        [&](http_request const& request, http_response& response, path_parameters_type const& parameters, http_context const& context)
        {
            // Just for convenience
            using namespace inja;
            using std::make_pair;
            using std::string;
            namespace fs = std::filesystem;

            std::string html;
            bool user_was_registered_with_success = false;
            bool user_was_registered_with_fail = false;
            std::string user_register_fail_reason = "";
            std::string username;

            if(request.method() == http_method::post)
            {
                std::cout << "Post method was called\n";

                using namespace boost;

                std::string s = request.body();
                boost::algorithm::trim(s);

                auto final_s = urlhelper::URLDecode(s);

                std::map<std::string, std::string> user_data;

                std::vector<std::string> key_values;
                boost::split(key_values, final_s, boost::is_any_of("&"));

                for(auto const& item : key_values)
                {
                    std::vector<std::string> key_value;
                    boost::split(key_value, item, boost::is_any_of("="));
                    if(key_value.size() == 2)
                    {
                        std::string key = key_value[0];
                        std::string value = key_value[1];
                        boost::algorithm::trim(key);
                        boost::algorithm::trim(value);

                        std::cout << key << " = " << value << std::endl;

                        user_data.insert(make_pair(key, value));
                    }
                }

                if(!user_data.contains("user_is_active"))
                    user_data.insert(make_pair("user_is_active", "off"));
                else if(!user_data.contains("user_is_staff_status"))
                    user_data.insert(make_pair("user_is_staff_status", "off"));
                else if(!user_data.contains("user_is_superuser_status"))
                    user_data.insert(make_pair("user_is_superuser_status", "off"));


                try 
                {
                    User u;
                    u.username = user_data["username"];
                    u.password = user_data["password"];
                    u.first_name = user_data["first_name"];
                    u.last_name = user_data["last_name"];
                    u.email = user_data["email"];
                    u.user_active = user_data["user_is_active"] == "on" ? true : false;
                    u.user_staff_status = user_data["user_is_staff_status"] == "on" ? true : false;
                    u.user_superuser_status = user_data["user_is_superuser_status"] == "on" ? true : false;

                    argon2d_password_hashing pass_hash;
                    u.password = pass_hash.generate_encoded_digest(u.password);

                    orm::insert(u, *database::global_connection->connection_ptr());

                    user_was_registered_with_success = true;

                    username = u.username;

                    std::cout << "User was registered with success\n";

                } 
                catch (std::exception const& ex)
                {
                    std::cout << "Error " << ex.what() << "\n";
                    user_register_fail_reason = ex.what();
                    user_was_registered_with_fail = true;
                }

            }

            try
            {
                response.add_header("content-type", "text/html; charset=UTF-8");

                auto current_path = fs::current_path().string();

                std::cout << "Current paht is " << current_path << "\n";

                Environment env;
                Template sidebar = env.parse_template("static/templates/admin/sidebar.html");
                env.include_template("sidebar", sidebar);
                Template temp = env.parse_template("static/templates/admin/user_add.html");

                auto users_data = this->many_serialized("Users");

                std::cout << users_data.dump() << "\n";

                json data;
                data["users"] = users_data;
                data["user_was_registered_with_success"] = user_was_registered_with_success;
                data["user_register_fail_reason"] = user_register_fail_reason;
                data["username"] = username;
                data["user_was_registered_with_fail"] = user_was_registered_with_fail;

                html = env.render(temp, data);

            }
            catch(std::exception const& ex)
            {
                std::cout << ex.what() << "\n";
            }

            response.set_body(html);
            response.set_status(http_status::ok);
        };

        auto admin_index_users_edit = 
        [&](http_request const& request, http_response& response, path_parameters_type const& parameters, http_context const& context)
        {
            // Just for convenience
            using namespace inja;
            using std::make_pair;
            using std::string;
            namespace fs = std::filesystem;

            std::string html;
            bool user_was_edited_with_success = false;
            bool user_was_edited_with_fail = false;
            std::string user_edit_fail_reason = "";
            std::string username;

            auto user_p = get_parameter(parameters, "user_p");
            if(!user_p)
            {
                response.set_status(http_status::bad_request);
                return;
            }

            if(request.method() == http_method::post)
            {
                std::cout << "Post method was called\n";

                using namespace boost;

                std::string s = request.body();
                boost::algorithm::trim(s);

                std::map<std::string, std::string> user_data;

                auto final_s = urlhelper::URLDecode(s);

                std::vector<std::string> key_values;
                boost::split(key_values, final_s, boost::is_any_of("&"));

                for(auto const& item : key_values)
                {
                    std::vector<std::string> key_value;
                    boost::split(key_value, item, boost::is_any_of("="));
                    if(key_value.size() == 2)
                    {
                        std::string key = key_value[0];
                        std::string value = key_value[1];
                        boost::algorithm::trim(key);
                        boost::algorithm::trim(value);

                        std::cout << key << " = " << value << std::endl;

                        user_data.insert(make_pair(key, value));
                    }
                }

                if(!user_data.contains("user_is_active"))
                    user_data.insert(make_pair("user_is_active", "off"));
                else if(!user_data.contains("user_is_staff_status"))
                    user_data.insert(make_pair("user_is_staff_status", "off"));
                else if(!user_data.contains("user_is_superuser_status"))
                    user_data.insert(make_pair("user_is_superuser_status", "off"));


                try 
                {
                    auto user_db = orm::select<User>(
                    fmt::format("WHERE username = '{}'", user_p->value),
                    *database::global_connection->connection_ptr());

                    if(!user_db)
                    {
                        make_internal_server_error_response(response, "User not found");
                        return;
                    }

                    if(user_data["password"] != "passwordnotshowed")
                    {
                        argon2d_password_hashing pass_hash;
                        user_db->password = pass_hash.generate_encoded_digest(user_data["password"]);
                    }

                    user_db->first_name = user_data["first_name"];
                    user_db->last_name = user_data["last_name"];
                    user_db->email = user_data["email"];
                    user_db->user_active = user_data["user_is_active"] == "on" ? true : false;
                    user_db->user_staff_status = user_data["user_is_staff_status"] == "on" ? true : false;
                    user_db->user_superuser_status = user_data["user_is_superuser_status"] == "on" ? true : false;
                    

                    auto result = orm::update<User>(
                        fmt::format("WHERE username = '{}'", user_db->username),
                        *database::global_connection->connection_ptr(),
                        make_pair("first_name", user_db->first_name),
                        make_pair("last_name", user_db->last_name),
                        make_pair("email", user_db->email),
                        make_pair("user_active", user_db->user_active),
                        make_pair("user_staff_status", user_db->user_staff_status),
                        make_pair("user_superuser_status", user_db->user_superuser_status),
                        make_pair("password", user_db->password)
                    );

                    user_was_edited_with_success = true;

                    if(result)
                        std::cout << "User was edited with success\n";

                } 
                catch (std::exception const& ex)
                {
                    std::cout << "Error " << ex.what() << "\n";
                    user_edit_fail_reason = ex.what();
                    user_was_edited_with_fail = true;
                }

            }    
            else if(request.method() == http_method::delete_)
            {
                std::cout << "Delete was called";

                auto result = orm::delete_<User>(
                    fmt::format("WHERE username = '{}'", user_p->value),
                    *database::global_connection->connection_ptr());

                if(!result)
                {
                    std::cout << "Error on delete user\n";
                }
            }

            try
            {
                response.add_header("content-type", "text/html; charset=UTF-8");

                auto current_path = fs::current_path().string();

                std::cout << "Current path is " << current_path << "\n";

                auto user_db = orm::select<User>(
                    fmt::format("WHERE username = '{}'", user_p->value),
                    *database::global_connection->connection_ptr());

                if(!user_db)
                {
                    make_internal_server_error_response(response, "User not found");
                    return;
                }

                Environment env;
                Template sidebar = env.parse_template("static/templates/admin/sidebar.html");
                env.include_template("sidebar", sidebar);
                Template temp = env.parse_template("static/templates/admin/user_edit.html");

                auto user_serialized = serialize(*user_db);

                std::cout << user_serialized.dump() << "\n";

                json data;
                data["user"] = user_serialized;
                data["user_was_edited_with_success"] = user_was_edited_with_success;
                data["user_edit_fail_reason"] = user_edit_fail_reason;
                data["username"] = user_db->username;
                data["user_was_edited_with_fail"] = user_was_edited_with_fail;

                html = env.render(temp, data);

            }
            catch(std::exception const& ex)
            {
                std::cout << ex.what() << "\n";
            }

            response.set_body(html);
            response.set_status(http_status::ok);
        };

        auto index_admin_models_list =
        [&](http_request const& request, http_response& response, path_parameters_type const& parameters, http_context const& context)
        {
            // Just for convenience
            using namespace inja;
            using std::make_pair;
            namespace fs = std::filesystem;

            std::string html;

            try
            {

                response.add_header("content-type", "text/html; charset=UTF-8");

                auto current_path = fs::current_path().string();

                std::cout << "Current paht is " << current_path << "\n";

                Environment env;
                Template sidebar = env.parse_template("static/templates/admin/sidebar.html");
                env.include_template("sidebar", sidebar);
                Template temp = env.parse_template("static/templates/admin/models_list.html");

                auto models = this->models_names();

                auto user_pos = std::find(models.begin(), models.end(), "Users");
                if(user_pos != models.end())
                    models.erase(user_pos);

                json data;
                data["models"] = models;

                html = env.render(temp, data);

            }
            catch(std::exception const& ex)
            {
                std::cout << ex.what() << "\n";
            }

            response.set_body(html);
            response.set_status(http_status::ok);
        };

        auto index_model_admin_detail = 
        [&](http_request const& request, http_response& response, path_parameters_type const& parameters, http_context const& context)
        {
            // Just for convenience
            using namespace inja;
            using std::make_pair;
            namespace fs = std::filesystem;

            auto model_name_p = get_parameter(parameters, "model_name");
            if(!model_name_p)
            {
                make_badrequest_response(response, "Model name not found");
                return;
            }

            std::string html;

            try
            {
                response.add_header("content-type", "text/html; charset=UTF-8");

                auto current_path = fs::current_path().string();

                std::cout << "Current paht is " << current_path << "\n";

                Environment env;

                env.add_callback("get_field_is_admin_identifier", 1, 
                [](Arguments& args) {
                    auto field_name = args.at(0)->get<std::string>();
                    return field_name == "username" || field_name == "email" || field_name == "first_name" || field_name == "last_name";
                });

                Template sidebar = env.parse_template("static/templates/admin/sidebar.html");
                env.include_template("sidebar", sidebar);
                Template temp = env.parse_template("static/templates/admin/model_detail.html");

                auto model_serialized = this->many_serialized(model_name_p->value);
                json data;
                data["model_list"] = model_serialized;
                data["model_name"] = model_name_p->value;

                std::cout << "Result serialized " << model_serialized.dump() << "\n";

                html = env.render(temp, data);

            }
            catch(std::exception const& ex)
            {
                std::cout << ex.what() << "\n";
            }

            response.set_body(html);
            response.set_status(http_status::ok);
        };

        auto index_admin_model_add = 
        [&](http_request const& request, http_response& response, path_parameters_type const& parameters, http_context const& context)
        {
            // Just for convenience
            using namespace inja;
            using std::make_pair;

            auto model_name_p = get_parameter(parameters, "model_name_p");
            if(!model_name_p)
            {
                make_badrequest_response(response, "Model name not found");
                return;
            }

            std::string html;
            bool model_was_registered_with_success = false;
            bool model_was_registered_with_fail = false;
            std::string model_register_fail_reason;

            if(request.method() == http_method::post)
            {
                std::cout << "Post method was called\n";

                using namespace boost;

                std::string s = request.body();
                boost::algorithm::trim(s);

                std::map<std::string, std::string> user_data;

                auto final_s = urlhelper::URLDecode(s);

                std::vector<std::string> key_values;
                boost::split(key_values, final_s, boost::is_any_of("&"));

                for(auto const& item : key_values)
                {
                    std::vector<std::string> key_value;
                    boost::split(key_value, item, boost::is_any_of("="));
                    if(key_value.size() == 2)
                    {
                        std::string key = key_value[0];
                        std::string value = key_value[1];
                        boost::algorithm::trim(key);
                        boost::algorithm::trim(value);

                        std::cout << key << " = " << value << std::endl;

                        user_data.insert(make_pair(key, value));
                    }
                }

                if(!this->insert_model(user_data, model_name_p->value))
                {
                    std::cout << "Failed to insert model\n";
                    model_was_registered_with_fail = true;
                    model_register_fail_reason = "Failed to insert model";
                }
                else
                    model_was_registered_with_success = true;
            }    

            auto x = this->get_typed_serialized(model_name_p->value);

            std::cout << "Result is " << x.dump() << "\n";

            Environment env;
            Template sidebar = env.parse_template("static/templates/admin/sidebar.html");
            env.include_template("sidebar", sidebar);
            Template temp = env.parse_template("static/templates/admin/model_detail_item_add.html");

            json data;
            data["fields"] = x;
            data["model_name"] = model_name_p->value;
            data["model_was_registered_with_success"] = model_was_registered_with_success;
            data["model_was_registered_with_fail"] = model_was_registered_with_fail;
            data["model_register_fail_reason"] = model_register_fail_reason;

            html = env.render(temp, data);

            response.add_header("content-type", "text/html; charset=UTF-8");
            response.set_body(html);
            response.set_status(http_status::ok);
        };

        auto index_admin_model_edit = 
        [&](http_request const& request, http_response& response, path_parameters_type const& parameters, http_context const& context)
        {
            // Just for convenience
            using namespace inja;
            using std::make_pair;

            auto model_name_p = get_parameter(parameters, "model_name_p");
            if(!model_name_p)
            {
                make_badrequest_response(response, "Model name not found");
                return;
            }

            auto model_name_pk = get_parameter(parameters, "model_item_id_pk");
            if(!model_name_pk)
            {
                make_badrequest_response(response, "model pk not found");
                return;
            }

            std::string html;
            bool model_was_edited_with_success = false;
            bool model_was_edited_with_fail = false;
            std::string model_edit_fail_reason;

            if(request.method() == http_method::post)
            {
                std::cout << "Post method was called\n";

                using namespace boost;

                std::string s = request.body();
                boost::algorithm::trim(s);

                std::unordered_map<std::string, std::string> model_data;

                auto final_s = urlhelper::URLDecode(s);

                std::vector<std::string> key_values;
                boost::split(key_values, final_s, boost::is_any_of("&"));

                for(auto const& item : key_values)
                {
                    std::vector<std::string> key_value;
                    boost::split(key_value, item, boost::is_any_of("="));
                    if(key_value.size() == 2)
                    {
                        std::string key = key_value[0];
                        std::string value = key_value[1];
                        boost::algorithm::trim(key);
                        boost::algorithm::trim(value);

                        std::cout << key << " = " << value << std::endl;

                        model_data.insert(make_pair(key, value));
                    }
                }

                if(!this->update_model(model_name_p->value, model_data, model_name_pk->value))
                {
                    model_was_edited_with_fail = true;
                    model_edit_fail_reason = "Failed to update model";
                }
                else
                    model_was_edited_with_success = true;
            }
            else if(request.method() == http_method::delete_)
            {
                std::cout << "Delete method was called\n";
                
                if(!this->delete_model(model_name_p->value, model_name_pk->value))
                {
                    std::cout << "Error on delete model " << model_name_p->value << "\n";
                }

            }

            auto x = this->get_model_typed_serialized(model_name_p->value, model_name_pk->value);

            Environment env;
            Template sidebar = env.parse_template("static/templates/admin/sidebar.html");
            env.include_template("sidebar", sidebar);
            Template temp = env.parse_template("static/templates/admin/model_detail_item_edit.html");

            json data;
            data["model"] = x;
            data["model_name"] = model_name_p->value;
            data["model_was_edited_with_success"] = model_was_edited_with_success;
            data["model_was_edited_with_fail"] = model_was_edited_with_fail;
            data["model_edit_fail_reason"] = model_edit_fail_reason;

            html = env.render(temp, data);

            response.add_header("content-type", "text/html; charset=UTF-8");
            response.set_body(html);
            response.set_status(http_status::ok);
        };

        routes_.push_back(make_shared<route>(http_method::get, "/admin/add/users", admin_index_users_add));
        routes_.push_back(make_shared<route>(http_method::post, "/admin/add/users", admin_index_users_add));
        routes_.push_back(make_shared<route>(http_method::get, "/admin/edit/users/:user_p", admin_index_users_edit));
        routes_.push_back(make_shared<route>(http_method::post, "/admin/edit/users/:user_p", admin_index_users_edit));
        routes_.push_back(make_shared<route>(http_method::delete_, "/admin/delete/users/:user_p", admin_index_users_edit));

        routes_.push_back(make_shared<route>(http_method::get, "/admin/models", index_admin_models_list));
        routes_.push_back(make_shared<route>(http_method::get, "/admin/models/:model_name", index_model_admin_detail));
        routes_.push_back(make_shared<route>(http_method::get, "/admin/models_add/:model_name_p", index_admin_model_add));
        routes_.push_back(make_shared<route>(http_method::post, "/admin/models_add/:model_name_p", index_admin_model_add));
        routes_.push_back(make_shared<route>(http_method::get, "/admin/models_edit/:model_name_p/:model_item_id_pk", index_admin_model_edit));
        routes_.push_back(make_shared<route>(http_method::post, "/admin/models_edit/:model_name_p/:model_item_id_pk", index_admin_model_edit));
        routes_.push_back(make_shared<route>(http_method::delete_, "/admin/models_delete/:model_name_p/:model_item_id_pk", index_admin_model_edit));
    }

private:
    std::vector<std::string> models_names()
    {
        using namespace refl;
        std::vector<std::string> result;

        orm::my_for_each(t, 
        [&result](auto arg, size_t index)
        {
            using Td = type_descriptor<decltype(arg)>;
            auto tbl = descriptor::get_attribute<orm::Table>(Td{});

            result.push_back(tbl.name);
        });

        return result;
    }

    std::any get_type(std::string const& model_name)
    {
        using namespace refl;

        std::any result;

        orm::my_for_each(t, 
        [&result, &model_name](auto arg, size_t index)
        {
            using Td = type_descriptor<decltype(arg)>;
            auto tbl = descriptor::get_attribute<orm::Table>(Td{});

            if(tbl.name == model_name)
                result = arg;
        });
        
        return result;
        
        //auto z = visit_at(t, 0, f2); // 42
        //auto xa = runtime_get(t, type_index_found);
    }
    
    nlohmann::json get_simple_serialized(std::string const& model_name)
    {
        using nlohmann::json;
        using namespace refl;
        auto make_vector_t = []<typename T>(T arg) -> std::vector<T>
        {
            return orm::select_many<T>("", *database::global_connection->connection_ptr());
        };

        auto type_finded = get_type(model_name);

        nlohmann::json data;

        orm::my_for_each(t,
        [&data, &type_finded, &make_vector_t](auto arg, size_t index)
        {
            using Td = type_descriptor<decltype(arg)>;

            if(type_finded.type() == typeid(arg))
            {
                auto ut = std::any_cast<decltype(arg)>(arg);
                auto res = make_vector_t(ut);

                data = serialize_admin(res);
            }

        });

        return data;
    }

    nlohmann::json many_serialized(std::string const& model_name)
    {
        using nlohmann::json;
        using namespace refl;
        auto make_vector_t = []<typename T>(T arg) -> std::vector<T>
        {
            return orm::select_many<T>("", *database::global_connection->connection_ptr());
        };

        std::any type_finded = get_type(model_name);

        nlohmann::json data;

        orm::my_for_each(t,
        [&data, &type_finded, &make_vector_t, &model_name](auto arg, size_t index)
        {
            using Td = type_descriptor<decltype(arg)>;

            if(type_finded.type() == typeid(arg))
            {
                auto ut = std::any_cast<decltype(arg)>(arg);
                auto res = make_vector_t(ut);

                // bem porco
                for(auto const& item : res)
                {
                    std::vector<ModelDataField> item_fields;
                    util::for_each(Td::members,
                    [&item, &item_fields, &model_name](auto member, auto index)
                    {
                        constexpr auto column = descriptor::get_attribute<orm::Column>(decltype(member){});
                        using MT = typename decltype(member)::value_type;

                        auto field_name = column.name;
                        auto field_type = get_type_name_from_model(member);
                        auto field_value = serializator::normalize_type(refl::runtime::invoke<MT>(item, field_name));
                        auto is_admin_field_identifier = column.is_admin_field_identifier;
                        
                        item_fields.push_back(ModelDataField(field_name, field_type, field_value, is_admin_field_identifier));
                    });

                    json obj;
                    auto fields = serialize_admin(item_fields);

                    obj["model_name"] = model_name;
                    obj["fields"] = fields;;

                    data.push_back(obj);
                }
            }

        });

        if(data.is_null())
        {
            data = json::array_t();
            //data["model_name"] = model_name;
            //data["fields"] = serialize_admin(std::vector<ModelDataField>{});
        }

        return data;
    }

    nlohmann::json get_model_typed_serialized(std::string const& model_name, std::string const& pk)
    {
        using nlohmann::json;
        using namespace refl;
        using webcrown::orm::DataType;
        auto get_type_db = []<typename T>(T arg, std::string const& cond) -> std::optional<T>
        {
            return orm::select<T>(cond, *database::global_connection->connection_ptr());
        };

        auto type_finded = get_type(model_name);

        nlohmann::json data;

        orm::my_for_each(t,
        [&data, &type_finded, &get_type_db, &model_name, &pk](auto arg, size_t index)
        {
            using Td = type_descriptor<decltype(arg)>;

            if(type_finded.type() == typeid(arg))
            {
                auto ut = std::any_cast<decltype(arg)>(arg);
                
                ModelData result{};

                // bem porco

                std::string pk_name;
                
                std::vector<ModelDataField> item_fields;
                util::for_each(Td::members,
                [&item_fields, &model_name, &pk_name](auto member, auto index)
                {
                    constexpr auto column = descriptor::get_attribute<orm::Column>(decltype(member){});
                    using MT = typename decltype(member)::value_type;

                    if constexpr (DataType::primarykey ==  column.data_type)
                    {
                        pk_name = column.name;
                    }
                });

                auto res = get_type_db(
                            ut, 
                            fmt::format("WHERE {} = {}", pk_name, pk));

                if(!res)
                    return;

                util::for_each(Td::members,
                [&item_fields, &model_name, &res](auto member, auto index)
                {
                    constexpr auto column = descriptor::get_attribute<orm::Column>(decltype(member){});
                    using MT = typename decltype(member)::value_type;

                    auto field_name = column.name;
                    auto field_type = get_type_name_from_model(member);
                    auto field_value = serializator::normalize_type(refl::runtime::invoke<MT>(*res, field_name));
                    auto is_admin_field_identifier = column.is_admin_field_identifier;
                
                    item_fields.push_back(ModelDataField(field_name, field_type, field_value, is_admin_field_identifier));
                });

                auto fields = serialize_admin(item_fields);

                data["model_name"] = model_name;
                data["fields"] = fields;
            }

        });

        return data;
    }

    std::optional<std::string> get_model_field_identifier(std::string const& model_name)
    {
        using namespace refl;

        std::string result;
        auto type_finded = get_type(model_name);

        orm::my_for_each(t,
        [&result, &type_finded](auto arg, size_t index)
        {
            using Td = type_descriptor<decltype(arg)>;

            if(type_finded.type() == typeid(arg))
            {
                util::for_each(Td::members,
                [&result](auto member, auto index)
                {
                    constexpr auto column = descriptor::get_attribute<orm::Column>(decltype(member){});
                    using MT = typename decltype(member)::value_type;

                    if(column.is_admin_field_identifier)
                    {
                        result = column.name;
                    }
                });
            }
        });

        if(!result.empty())
            return result;

        return std::nullopt;
    }

    nlohmann::json get_typed_serialized(std::string const& model_name)
    {
        using nlohmann::json;
        using std::make_pair;
        using namespace refl;
        using webcrown::orm::DataType;
        using webcrown::orm::ColumnFlags;

        auto type_finded = get_type(model_name);
        std::vector<ModelDescriptionItem> field_mapped;

        orm::my_for_each(t,
        [&type_finded, &field_mapped](auto arg, size_t index)
        {
            using Td = type_descriptor<decltype(arg)>;

            if(type_finded.type() == typeid(arg))
            {
                util::for_each(Td::members,
                [&field_mapped](auto member, auto index)
                {
                    constexpr auto column = descriptor::get_attribute<orm::Column>(decltype(member){});
                    using MT = typename decltype(member)::value_type;

                    if constexpr (DataType::primarykey == column.data_type)
                    {
                        return;
                    }

                    if constexpr (
                        DataType::bigserial == column.data_type ||
                        DataType::integer == column.data_type ||
                        DataType::bigint == column.data_type ||
                        DataType::smallint == column.data_type ||
                        DataType::smallserial == column.data_type ||
                        DataType::serial == column.data_type)
                    {
                        constexpr bool is_integral = std::is_integral_v<MT>;
                        static_assert(is_integral && "Type should be an integral type like int");

                        field_mapped.push_back(ModelDescriptionItem{column.name, "integer"});
                    }
                    else if constexpr (DataType::text == column.data_type)
                    {
                        constexpr bool is_text = std::is_same_v<MT, std::string> || std::is_same_v<MT, const char*>;
                        static_assert(is_text && "Type should be a text type like std::string, const char*");

                        field_mapped.push_back(ModelDescriptionItem{column.name, "text"});
                    }
                    else if constexpr (DataType::boolean == column.data_type)
                    {
                        constexpr bool is_bool = std::is_same_v<MT, bool>;
                        static_assert(is_bool && "Type should be boolean");

                        field_mapped.push_back(ModelDescriptionItem{column.name, "boolean"});
                    }
                    else if constexpr (DataType::date == column.data_type)
                    {
                        constexpr auto is_date = std::is_same_v<MT, date::year_month_day>;
                        static_assert(is_date && "Type should be a date");

                        field_mapped.push_back(ModelDescriptionItem{column.name, "date"});
                    }
                    else if constexpr (DataType::decimal == column.data_type)
                    {
                        constexpr auto is_decimal = std::is_floating_point_v<MT>;
                        static_assert(is_decimal && "Type should be a floating pointer");

                        field_mapped.push_back(ModelDescriptionItem{column.name, "decimal"});
                    }
                    else if constexpr (DataType::timestamp == column.data_type)
                    {
                        constexpr auto is_timestamp = std::is_same_v<MT, date_time>;
                        static_assert (is_timestamp && "Type should be date_time");

                        field_mapped.push_back(ModelDescriptionItem{column.name, "timestamp"});
                    }
                    else if constexpr (DataType::time == column.data_type)
                    {
                        constexpr auto is_time = std::is_same_v<MT, date::time_of_day<std::chrono::seconds>>;
                        static_assert (is_time && "Type should be date::time_of_day<std::chrono::seconds>");

                        field_mapped.push_back(ModelDescriptionItem{column.name, "time"});
                    }
                });

                
            }

        });
        
        
        return serialize_admin(field_mapped);
    }

    bool insert_model(std::map<std::string, std::string> const& data, std::string const& model_name)
    {
        using namespace refl;
        auto type_finded = get_type(model_name);

        auto insert_ = []<typename T>(T arg) -> bool
        {
            return orm::insert<T>(arg, *database::global_connection->connection_ptr());
        };

        bool result{};

        orm::my_for_each(t,
        [&type_finded, &data, &result, &insert_](auto arg, size_t index)
        {
            using Td = type_descriptor<decltype(arg)>;

            if(type_finded.type() == typeid(arg))
            {
                auto ut = std::any_cast<decltype(arg)>(arg);

                util::for_each(Td::members,
                [&data, &result, &ut, &insert_](auto member, auto index)
                {
                    using MT = typename decltype(member)::value_type;
                    constexpr auto column = descriptor::get_attribute<orm::Column>(decltype(member){});
                    const char* fname = refl::descriptor::get_display_name(member);

                    if constexpr (orm::DataType::primarykey == column.data_type)
                    {
                        return;
                    }

                    auto v = boost::lexical_cast<MT>(data.at(fname));

                    refl::runtime::invoke<MT>(ut, fname, v);
                    
                });

                result = insert_(ut);
            }
        });

        return result;
    }

    bool delete_model(std::string const& model_name, std::string const& pk_value)
    {
        using namespace refl;
        auto type_finded = get_type(model_name);

        auto mdelete_ = []<typename T>(T arg, std::string const& cond) -> bool
        {
            return orm::delete_<T>(cond, *database::global_connection->connection_ptr());
        };

        bool result{};

        orm::my_for_each(t,
        [&type_finded, &result, &mdelete_, &pk_value](auto arg, size_t index)
        {
            using Td = type_descriptor<decltype(arg)>;

            if(type_finded.type() == typeid(arg))
            {
                auto ut = std::any_cast<decltype(arg)>(arg);

                std::string pk_name;

                util::for_each(Td::members,
                [&pk_name](auto member, auto index)
                {
                    using MT = typename decltype(member)::value_type;
                    constexpr auto column = descriptor::get_attribute<orm::Column>(decltype(member){});
                    const char* fname = refl::descriptor::get_display_name(member);

                    if constexpr (orm::DataType::primarykey == column.data_type)
                    {
                        pk_name = column.name;
                    }
                });
                
                result = mdelete_(
                    ut,
                    fmt::format("WHERE {} = '{}'", pk_name, pk_value));
            }
        });

        return result;
    }

    bool update_model(std::string const& model_name, std::unordered_map<std::string, std::string> const& data, std::string const& pk_value)
    {
        using namespace refl;
        using std::string;
        using std::unordered_map;

        auto type_finded = get_type(model_name);

        auto update_ = [&]<typename T>(T arg, string const& pk_name) -> bool
        {
            return orm::update<T>(
                fmt::format("WHERE {} = '{}'", pk_name, pk_value),
                *database::global_connection->connection_ptr(),
                data
            );
        };

        bool result{};

        orm::my_for_each(t,
        [&type_finded, &result, &update_](auto arg, size_t index)
        {
            using Td = type_descriptor<decltype(arg)>;

            if(type_finded.type() == typeid(arg))
            {
                auto ut = std::any_cast<decltype(arg)>(arg);

                string pk_name;
                util::for_each(Td::members,
                [&pk_name](auto member, auto index)
                {
                    constexpr auto column = descriptor::get_attribute<orm::Column>(decltype(member){});
                    using MT = typename decltype(member)::value_type;

                    if constexpr (orm::DataType::primarykey ==  column.data_type)
                    {
                        pk_name = column.name;
                    }
                });

                result = update_(ut, pk_name);
            }
        });

        return result;
    }
};

}}
