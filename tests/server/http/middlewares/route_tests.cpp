#include <gmock/gmock.h>
#include "webcrown/server/http/middlewares/route.hpp"
#include "webcrown/server/http/http_parser.hpp"
#include <memory>

TEST(HTTP_REQUEST_TESTS, mach_target_request_single_parameter_should_return_expected_param_value)
{
    using webcrown::server::http::route;
    using webcrown::server::http::parser;
    using webcrown::server::http::http_method;

    // Expected
    std::string const expected_username_value = "aelxtpt";

    // Scenario
    char const* raw_buffer = "GET /user/aelxtpt HTTP/1.1\r\nUser-Agent: PostmanRuntime/7.26.10\r\nAccept: */*\r\nPostman-Token: e8047ed2-032e-47fb-92e8-5910f460600e\r\nHost: localhost:8443\r\nAccept-Encoding: gzip, deflate, br\r\nConnection: keep-alive\r\n\r\n";
    route rt(http_method::get, "/user/:username");

    std::error_code ec;
    parser parser;
    auto http_request = parser.parse_start_line(raw_buffer, std::strlen(raw_buffer), ec);

    rt.is_match_with_target_request(http_request->target());

    // Assert
    ASSERT_TRUE(http_request);
    ASSERT_EQ(expected_username_value, rt.path_parameters()[0].second);
}

// Method scenario expected
TEST(ROUTE_TESTS, parse_single_parameter_path_should_return_expected_param_value)
{
    using webcrown::server::http::route;
    using webcrown::server::http::parser;
    using webcrown::server::http::http_method;

    // Expected
    std::string const expected_param_value = "username";

    // Scenario
    route rt(http_method::get, "/user/:username");
    auto result = rt.path_parameters();

    // Assert
    ASSERT_TRUE(result.size() == 1);
    ASSERT_EQ(expected_param_value, result[0].first);
}

TEST(ROUTE_TESTS, parse_two_parameters_path_should_return_expected_param_values)
{
    using webcrown::server::http::route;
    using webcrown::server::http::parser;
    using webcrown::server::http::http_method;

    // Expected
    std::string const expected_param_value_one = "username";
    std::string const expected_param_value_two = "games";

    // Scenario
    route rt(http_method::get, "/user/:username/:games");
    auto result = rt.path_parameters();

    // Assert
    ASSERT_TRUE(result.size() == 2);
    ASSERT_EQ(expected_param_value_one, result[0].first);
    ASSERT_EQ(expected_param_value_two, result[1].first);
}

TEST(ROUTE_TESTS, parse_single_parameter_path_with_end_slash_should_return_expected_param_value)
{
    using webcrown::server::http::route;
    using webcrown::server::http::parser;
    using webcrown::server::http::http_method;

    // Expected
    std::string const expected_param_value = "username";

    // Scenario
    route rt(http_method::get, "/profile/:username/");
    auto result = rt.path_parameters();

    // Assert
    ASSERT_TRUE(result.size() == 1);
    ASSERT_EQ(expected_param_value, result[0].first);
}

TEST(ROUTE_TESTS, parse_two_parameters_path_with_end_slash_should_return_expected_param_values)
{
    using webcrown::server::http::route;
    using webcrown::server::http::http_method;

    // Expected
    std::string const expected_param_value_one = "username";
    std::string const expected_param_value_two = "games";

    // Scenario
    route rt(http_method::get, "/user/:username/:games/");
    auto result = rt.path_parameters();

    // Assert
    ASSERT_TRUE(result.size() == 2);
    ASSERT_EQ(expected_param_value_one, result[0].first);
    ASSERT_EQ(expected_param_value_two, result[1].first);
}