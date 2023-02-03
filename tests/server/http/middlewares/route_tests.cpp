#include <gmock/gmock.h>
#include "webcrown/server/http/middlewares/route.hpp"
#include "webcrown/server/http/http_parser.hpp"
#include <memory>

TEST(ROUTE_TESTS, mach_target_request_single_parameter_should_return_expected_param_value)
{
    using webcrown::server::http::route;
    using webcrown::server::http::parser;
    using webcrown::server::http::http_method;
    using webcrown::server::http::http_request;
    using webcrown::server::http::http_response;

    // Expected
    std::string const expected_username_value = "aelxtpt";

    // Scenario
    char const* raw_buffer = "GET /user/aelxtpt HTTP/1.1\r\nUser-Agent: PostmanRuntime/7.26.10\r\nContent-Type: application/json\r\nAccept: */*\r\nPostman-Token: e8047ed2-032e-47fb-92e8-5910f460600e\r\nHost: localhost:8443\r\nAccept-Encoding: gzip, deflate, br\r\nConnection: keep-alive\r\n\r\n";
    route rt(http_method::get, "/user/:username", [](http_request const &request, http_response &response){});

    auto logger = spdlog::default_logger();

    std::error_code ec;
    parser parser(logger);
    auto http_request = parser.parse(raw_buffer, std::strlen(raw_buffer), ec);

    auto result = rt.is_match_with_target_request(http_request->target(), http_request->method());
    auto path_parameters_result = rt.path_parameters();


    // Assert
    ASSERT_TRUE(result);
    ASSERT_TRUE(path_parameters_result.size() == 1);
    ASSERT_EQ(expected_username_value, path_parameters_result[0].value);
}

TEST(ROUTE_TESTS, mach_target_request_two_parameters_should_return_expected_param_values)
{
    using webcrown::server::http::route;
    using webcrown::server::http::parser;
    using webcrown::server::http::http_method;
    using webcrown::server::http::http_request;
    using webcrown::server::http::http_response;

    // Expected
    std::string const expected_first_value = "aelxtpt";
    std::string const expected_second_value = "leagueoflegends";

    // Scenario
    char const* raw_buffer = "GET /user/profile/aelxtpt/leagueoflegends/edit HTTP/1.1\r\nUser-Agent: PostmanRuntime/7.26.10\r\nAccept: */*\r\nPostman-Token: e8047ed2-032e-47fb-92e8-5910f460600e\r\nHost: localhost:8443\r\nAccept-Encoding: gzip, deflate, br\r\nConnection: keep-alive\r\n\r\n";
    route rt(http_method::get, "/user/profile/:username/:game/edit", [](http_request const &request, http_response &response){});

    auto logger = spdlog::default_logger();

    std::error_code ec;
    parser parser(logger);
    auto http_request = parser.parse(raw_buffer, std::strlen(raw_buffer), ec);

    auto result = rt.is_match_with_target_request(http_request->target(), http_request->method());
    auto path_parameters_result = rt.path_parameters();

    // Assert
    ASSERT_TRUE(result);
    ASSERT_TRUE(path_parameters_result.size() == 2);
    ASSERT_EQ(expected_first_value, path_parameters_result[0].value);
    ASSERT_EQ(expected_second_value, path_parameters_result[1].value);
}

TEST(ROUTE_TESTS, mach_target_request_two_parameters_and_one_path_should_return_expected_param_values)
{
    using webcrown::server::http::route;
    using webcrown::server::http::parser;
    using webcrown::server::http::http_method;
    using webcrown::server::http::http_request;
    using webcrown::server::http::http_response;

    // Expected
    std::string const expected_first_value = "aelxtpt";
    std::string const expected_second_value = "leagueoflegends";

    // Scenario
    char const* raw_buffer = "GET /profile/aelxtpt/leagueoflegends HTTP/1.1\r\nUser-Agent: PostmanRuntime/7.26.10\r\nAccept: */*\r\nPostman-Token: e8047ed2-032e-47fb-92e8-5910f460600e\r\nHost: localhost:8443\r\nAccept-Encoding: gzip, deflate, br\r\nConnection: keep-alive\r\n\r\n";
    route rt(http_method::get, "/profile/:username/:game", [](http_request const &request, http_response &response){});

    auto logger = spdlog::default_logger();

    std::error_code ec;
    parser parser(logger);
    auto http_request = parser.parse(raw_buffer, std::strlen(raw_buffer), ec);

    auto result = rt.is_match_with_target_request(http_request->target(), http_request->method());
    auto path_parameters_result = rt.path_parameters();

    // Assert
    ASSERT_TRUE(result);
    ASSERT_TRUE(path_parameters_result.size() == 2);
    ASSERT_EQ(expected_first_value, path_parameters_result[0].value);
    ASSERT_EQ(expected_second_value, path_parameters_result[1].value);
}

TEST(ROUTE_TESTS, match_target_request_no_parameter_should_match_uri_target)
{
    using webcrown::server::http::route;
    using webcrown::server::http::parser;
    using webcrown::server::http::http_method;
    using webcrown::server::http::http_request;
    using webcrown::server::http::http_response;

    // Expected

    // Scenario
    char const* raw_buffer = "POST /user/register HTTP/1.1\r\nUser-Agent: PostmanRuntime/7.26.10\r\nAccept: */*\r\nPostman-Token: e8047ed2-032e-47fb-92e8-5910f460600e\r\nHost: localhost:8443\r\nAccept-Encoding: gzip, deflate, br\r\nConnection: keep-alive\r\n\r\n";
    route rt(http_method::post, "/user/register", [](http_request const &request, http_response &response){});

    auto logger = spdlog::default_logger();

    std::error_code ec;
    parser parser(logger);
    auto http_request = parser.parse(raw_buffer, std::strlen(raw_buffer), ec);

    auto result = rt.is_match_with_target_request(http_request->target(), http_request->method());
    auto path_parameters_result = rt.path_parameters();

    // Assert
    ASSERT_TRUE(result);
    ASSERT_TRUE(path_parameters_result.empty());
}

// Method scenario expected
TEST(ROUTE_TESTS, parse_single_parameter_path_should_return_expected_param_value)
{
    using webcrown::server::http::route;
    using webcrown::server::http::parser;
    using webcrown::server::http::http_method;
    using webcrown::server::http::http_request;
    using webcrown::server::http::http_response;

    // Expected
    std::string const expected_param_value = "username";

    // Scenario
    route rt(http_method::get, "/user/:username", [](http_request const &request, http_response &response){});
    auto result = rt.path_parameters();

    // Assert
    ASSERT_TRUE(result.size() == 1);
    ASSERT_EQ(expected_param_value, result[0].name);
}

TEST(ROUTE_TESTS, parse_no_parameter_should_return_expected_parameter_name)
{
    using webcrown::server::http::route;
    using webcrown::server::http::parser;
    using webcrown::server::http::http_method;
    using webcrown::server::http::http_request;
    using webcrown::server::http::http_response;

    // Expected
    std::string const expected_uri_target = "/user/register";

    // Scenario
    route rt(http_method::get, "/user/register", [](http_request const &request, http_response &response){});
    auto result = rt.uri_target();
    auto path_parameters_result = rt.path_parameters();

    // Assert
    ASSERT_EQ(expected_uri_target, result);
    ASSERT_TRUE(path_parameters_result.empty());
}

TEST(ROUTE_TESTS, parse_two_parameters_path_should_return_expected_param_values)
{
    using webcrown::server::http::route;
    using webcrown::server::http::parser;
    using webcrown::server::http::http_method;
    using webcrown::server::http::http_request;
    using webcrown::server::http::http_response;

    // Expected
    std::string const expected_param_value_one = "username";
    std::string const expected_param_value_two = "games";

    // Scenario
    route rt(http_method::get, "/user/:username/:games", [](http_request const &request, http_response &response){});
    auto result = rt.path_parameters();

    // Assert
    ASSERT_TRUE(result.size() == 2);
    ASSERT_EQ(expected_param_value_one, result[0].name);
    ASSERT_EQ(expected_param_value_two, result[1].name);
}

TEST(ROUTE_TESTS, parse_single_parameter_path_with_end_slash_should_return_expected_param_value)
{
    using webcrown::server::http::route;
    using webcrown::server::http::parser;
    using webcrown::server::http::http_method;
    using webcrown::server::http::http_request;
    using webcrown::server::http::http_response;

    // Expected
    std::string const expected_param_value = "username";

    // Scenario
    route rt(http_method::get, "/profile/:username/", [](http_request const &request, http_response &response){});
    auto result = rt.path_parameters();

    // Assert
    ASSERT_TRUE(result.size() == 1);
    ASSERT_EQ(expected_param_value, result[0].name);
}

TEST(ROUTE_TESTS, parse_two_parameters_path_with_end_slash_should_return_expected_param_values)
{
    using webcrown::server::http::route;
    using webcrown::server::http::http_method;
    using webcrown::server::http::http_request;
    using webcrown::server::http::http_response;

    // Expected
    std::string const expected_param_value_one = "username";
    std::string const expected_param_value_two = "games";

    // Scenario
    route rt(http_method::get, "/user/:username/:games/", [](http_request const &request, http_response &response){});
    auto result = rt.path_parameters();

    // Assert
    ASSERT_TRUE(result.size() == 2);
    ASSERT_EQ(expected_param_value_one, result[0].name);
    ASSERT_EQ(expected_param_value_two, result[1].name);
}

TEST(ROUTE_TESTS, parse_single_parameter_in_middle_route_should_return_expected_param_values)
{
    // using webcrown::server::http::route;
    // using webcrown::server::http::http_method;
    // using webcrown::server::http::http_request;
    // using webcrown::server::http::http_response;

    // // Expected
    // std::string const expected_param_value_one = "username";

    // // Scenario
    // route rt(http_method::get, "/user/profile/:username/cover_image/edit", [](http_request const &request, http_response &response){});
    // auto result = rt.path_parameters();
    // auto route_match = rt.is_match_with_target_request("/user/profile/aelxtpt/cover_image/edit", http_method::get);

    // // Assert
    // ASSERT_TRUE(result.size() == 1);
    // ASSERT_TRUE(route_match);
    // ASSERT_EQ(expected_param_value_one, result[0].name);
}

TEST(ROUTE_TESTS, routes_with_same_initial_path_should_match_only_the_exact_route)
{
    // using webcrown::server::http::route;
    // using webcrown::server::http::http_method;
    // using webcrown::server::http::http_request;
    // using webcrown::server::http::http_response;

    // // Expected

    // // Scenario
    // route first_rt(http_method::get, "/users/:username", [](http_request const& request, http_response& response){});
    // auto first_rt_match = first_rt.is_match_with_target_request("/users/aex4/images", http_method::get);

    // route second_rt(http_method::patch, "/users/:username/images", [](http_request const& request, http_response& response){});
    // auto second_rt_match = second_rt.is_match_with_target_request("/users/aex4/images", http_method::patch);

    // ASSERT_FALSE(first_rt_match);
    // ASSERT_TRUE(second_rt_match);
}