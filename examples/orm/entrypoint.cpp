#include "webcrown/orm/databases/postgres/enums.hpp"
#include "webcrown/orm/databases/postgres/postgres.hpp"
#include <cstdint>
#include <webcrown/webcrown.hpp>

#include <webcrown/orm/orm.hpp>
#include <webcrown/common/date/date_time.hpp>

using std::string;
using namespace webcrown;

namespace database {

using std::make_shared;
using std::shared_ptr;

static const std::string CONNECTION_STRING = "host=localhost port=5991 dbname=test connect_timeout=10 user=postgres password=Xpto@12";


class ConnectionGuard
{
  shared_ptr<pqxx::connection> connection_;
public:
  ConnectionGuard()
    {
    connection_ = make_shared<pqxx::connection>(CONNECTION_STRING);
  }

  pqxx::connection& connection() { return *connection_; }
  pqxx::connection* connection_ptr() { return connection_.get(); }
};

inline shared_ptr<ConnectionGuard> global_connection = make_shared<ConnectionGuard>();

}

struct User
{
    int64_t id{};
    string name{};
    int age{};
    double weight{};
    double height{};
    webcrown::date_time created_at{};
    date::time_of_day<std::chrono::seconds> updated_at{};

};

REFL_AUTO(
    type(User, orm::Table{"User"}),
    field(id, orm::Column{"id", orm::DataType::bigserial, 0, orm::ColumnFlags::primarykey | orm::ColumnFlags::ignore_insert}),
    field(name, orm::Column{"name", orm::DataType::text}),
    field(age, orm::Column{"age", orm::DataType::integer}),
    field(weight, orm::Column{"weight", orm::DataType::decimal}),
    field(height, orm::Column{"height", orm::DataType::decimal}),
    field(created_at, orm::Column{"created_at", orm::DataType::timestamp}),
    field(updated_at, orm::Column{"updated_at", orm::DataType::time})
);

int main()
{
    orm::create_table<User>(*database::global_connection->connection_ptr());
}