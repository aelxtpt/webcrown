#include <gmock/gmock.h>
#include "webcrown/server/http/parser.hpp"
#include "webcrown/server/http/detail/parser.hpp"
#include <memory>

//TEST(HTTP_PARSER, xpp)
//{
//    char const* raw = "GET / HTTP/1.1\r\nUser-Agent: PostmanRuntime/7.26.10\r\nAccept: */*\r\nPostman-Token: 073ac2a7-361c-4614-9f27-0ae50b879d57\r\nHost: localhost:8443\r\nAccept-Encoding: gzip, deflate, br\r\nConnection: keep-alive\r\n\r\n";
//
//    using webcrown::server::http::parser;
//    parser p{};
//
//    std::error_code ec{};
//
//    p.parse_start_line(raw, std::strlen(raw), ec);
//}


TEST(HTTP_PARSER, parse_method_with_invalid_characters_should_return_error)
{
    // Expected
    const std::string expected_error_msg = "http bad method";

    // Scenario
    char const* raw = "$@SER /";

    using webcrown::server::http::parser;
    parser p{};
    std::error_code ec{};
    std::string_view method;

    char const*& it = raw;
    char const* last = raw + std::strlen(raw);

    p.parse_method(it, last, method, ec);

    // Assert
    ASSERT_EQ(expected_error_msg, ec.message());
}

TEST(HTTP_PARSER, parse_method_with_incomplete_requestline_should_return_error)
{
    // Expected
    const std::string expected_error_msg = "http incomplete start line";

    // Scenario
    char const* raw = "GET";

    using webcrown::server::http::parser;
    parser p{};
    std::error_code ec{};
    std::string_view method;

    char const*& it = raw;
    char const* last = raw + std::strlen(raw);

    p.parse_method(it, last, method, ec);

    // Assert
    ASSERT_EQ(expected_error_msg, ec.message());
}

TEST(HTTP_PARSER, parse_method_with_method_without_space_should_return_error)
{
    // Expected
    const std::string expected_error_msg = "http bad method";

    // Scenario
    char const* raw = "GET/ ";

    using webcrown::server::http::parser;
    parser p{};
    std::error_code ec{};
    std::string_view method;

    char const*& it = raw;
    char const* last = raw + std::strlen(raw);

    p.parse_method(it, last, method, ec);

    // Assert
    ASSERT_EQ(expected_error_msg, ec.message());
}

TEST(HTTP_PARSER, parse_method_filled_with_space_string_should_return_error)
{
    // Expected
    const std::string expected_error_msg = "http bad method";

    // Scenario
    char const* raw = "  ";

    using webcrown::server::http::parser;
    parser p{};
    std::error_code ec{};
    std::string_view method;

    char const*& it = raw;
    char const* last = raw + std::strlen(raw);

    p.parse_method(it, last, method, ec);

    // Assert
    ASSERT_EQ(expected_error_msg, ec.message());
}

TEST(HTTP_PARSER, parse_method_with_two_spaces_after_should_return_expected_method)
{
    // This test tests the (increment) at the end of method

    // Expected
    const std::string expected_method = "GET";

    // Scenario
    char const* raw = "GET  ";

    using webcrown::server::http::parser;
    parser p{};
    std::error_code ec{};
    std::string_view method;

    char const*& it = raw;
    char const* last = raw + std::strlen(raw);

    p.parse_method(it, last, method, ec);

    // Assert
    ASSERT_EQ(expected_method, std::string(method));
}