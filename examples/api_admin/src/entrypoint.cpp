#include "asio/error_code.hpp"
#include "refl.hpp"
#include "webcrown/orm/databases/postgres/postgres.hpp"
#include "webcrown/server/http/http_method.hpp"
#include "webcrown/server/http/http_request.hpp"
#include "webcrown/server/http/http_response.hpp"
#include "webcrown/server/webserver.hpp"
#include "webcrown/server/http/middlewares/auth_middleware.hpp"
#include "webcrown/server/http/middlewares/cors/cors_middleware.hpp"
#include "webcrown/server/http/middlewares/route.hpp"
#include "webcrown/server/http/middlewares/routing_middleware.hpp"
#include "webcrown/server/http/status.hpp"
#include "webcrown/admin/admin.hpp"

#include <iostream>

using std::make_shared;
namespace http = webcrown::server::http;
using namespace webcrown::server;

const std::string CONNECTION_STRING = "host=localhost port=5432 dbname=test connect_timeout=10 user=postgres password=Xpto@12";

// Model
struct Course
{
    int id{};
    std::string name{};
    std::string description{};
};

REFL_AUTO(
    type(Course, webcrown::orm::Table{"Course"}),
    field(id, webcrown::orm::Column{"id", webcrown::orm::DataType::primarykey, 0, webcrown::orm::ColumnFlags::ignore_insert}),
    field(name, webcrown::orm::Column{"name", webcrown::orm::DataType::text, 0, webcrown::orm::ColumnFlags::none, "UNIQUE", true}),
    field(description, webcrown::orm::Column{"description", webcrown::orm::DataType::text})
);

void
home(http::http_request const& request, http::http_response& response, http::path_parameters_type const& parameters, http::http_context const& context)
{
    response.set_status(http::http_status::ok);
    response.set_body("Hello world");
}

void on_error(asio::error_code ec)
{
    std::cout << "Error " << ec.value() << "\n";
}

int main()
{
    try
    {
        auto connection = make_shared<pqxx::connection>(CONNECTION_STRING);
        webcrown::orm::create_table<Course>(*connection);

        auto http = make_shared<WebServer>("127.0.0.1", 8080, on_error);

        auto router_middleware = make_shared<http::routing_middleware>();
        auto cors_middleware = make_shared<http::cors_middleware>();

        router_middleware->add_router(make_shared<http::route>(http::http_method::get, "/", home));

        webcrown::admin::AdminManager<Course> admin_manager;
        admin_manager.register_models(connection, Course{});
        
        for(auto const& r : admin_manager.routes())
            router_middleware->add_router(r);

        http->add_middleware(cors_middleware);
        http->add_middleware(router_middleware);

        asio::error_code ec;
        http->start();

        while(!http->is_started())
            sched_yield();

        for(;;)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

        http->stop();
    }
    catch(std::exception const& ex)
    {
        printf("Error on server %s", ex.what());
    }
}
