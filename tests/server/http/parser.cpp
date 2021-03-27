#include <gmock/gmock.h>
#include "webcrown/server/http/parser.hpp"
#include "webcrown/server/http/detail/parser.hpp"
#include <memory>

TEST(HTTP_PARSER, parse_method_with_invalid_characters_should_return_error)
{
    using webcrown::server::http::parse_phase;
    using webcrown::server::http::parser;

    // Expected
    const std::string expected_error_msg = "http bad method";
    const parse_phase expected_parse_phase = parse_phase::parse_method;

    // Scenario
    char const* raw = "$@SER /";

    parser p{};
    std::error_code ec{};
    std::string_view method;

    char const*& it = raw;
    char const* last = raw + std::strlen(raw);

    p.parse_method(it, last, method, ec);

    // Assert
    ASSERT_EQ(expected_error_msg, ec.message());
    ASSERT_EQ(expected_parse_phase, p.parse_phase());
}

TEST(HTTP_PARSER, parse_method_with_incomplete_requestline_should_return_error)
{
    using webcrown::server::http::parse_phase;
    using webcrown::server::http::parser;

    // Expected
    const std::string expected_error_msg = "http incomplete start line";
    const parse_phase expected_parse_phase = parse_phase::parse_method;

    // Scenario
    char const* raw = "GET";

    parser p{};
    std::error_code ec{};
    std::string_view method;

    char const*& it = raw;
    char const* last = raw + std::strlen(raw);

    p.parse_method(it, last, method, ec);

    // Assert
    ASSERT_EQ(expected_error_msg, ec.message());
    ASSERT_EQ(expected_parse_phase, p.parse_phase());
}

TEST(HTTP_PARSER, parse_method_with_method_without_space_should_return_error)
{
    using webcrown::server::http::parse_phase;
    using webcrown::server::http::parser;

    // Expected
    const std::string expected_error_msg = "http bad method";
    const parse_phase expected_parse_phase = parse_phase::parse_method;

    // Scenario
    char const* raw = "GET/ ";

    parser p{};
    std::error_code ec{};
    std::string_view method;

    char const*& it = raw;
    char const* last = raw + std::strlen(raw);

    p.parse_method(it, last, method, ec);

    // Assert
    ASSERT_EQ(expected_error_msg, ec.message());
    ASSERT_EQ(expected_parse_phase, p.parse_phase());
}

TEST(HTTP_PARSER, parse_method_filled_with_space_string_should_return_error)
{
    using webcrown::server::http::parse_phase;
    using webcrown::server::http::parser;

    // Expected
    const std::string expected_error_msg = "http bad method";
    const parse_phase expected_parse_phase = parse_phase::parse_method;

    // Scenario
    char const* raw = "  ";

    parser p{};
    std::error_code ec{};
    std::string_view method;

    char const*& it = raw;
    char const* last = raw + std::strlen(raw);

    p.parse_method(it, last, method, ec);

    // Assert
    ASSERT_EQ(expected_error_msg, ec.message());
    ASSERT_EQ(expected_parse_phase, p.parse_phase());
}

TEST(HTTP_PARSER, parse_method_with_two_spaces_after_should_return_expected_method)
{
    // This test tests the (increment) at the end of function

    using webcrown::server::http::parse_phase;
    using webcrown::server::http::parser;

    // Expected
    const std::string expected_method = "GET";
    const parse_phase expected_parse_phase = parse_phase::parse_method_finished;

    // Scenario
    char const* raw = "GET  ";

    parser p{};
    std::error_code ec{};
    std::string_view method;

    char const*& it = raw;
    char const* last = raw + std::strlen(raw);

    p.parse_method(it, last, method, ec);

    // Assert
    ASSERT_EQ(expected_method, std::string(method));
    ASSERT_EQ(expected_parse_phase, p.parse_phase());
}

TEST(HTTP_PARSER, parse_target_with_space_at_begin_should_return_error)
{
    using webcrown::server::http::parse_phase;
    using webcrown::server::http::parser;

    // Expected
    const std::string expected_error_msg = "http bad target";
    const parse_phase expected_parse_phase = parse_phase::parse_target;

    // Scenario
    char const* raw = " user/1934";

    parser p{};
    std::error_code ec{};
    std::string_view target;

    char const*& it = raw;
    char const* last = raw + std::strlen(raw);

    p.parse_target(it, last, target, ec);

    // Assert
    ASSERT_EQ(expected_error_msg, ec.message());
    ASSERT_EQ(expected_parse_phase, p.parse_phase());
}

TEST(HTTP_PARSER, parse_target_with_incomplete_request_line_should_return_error)
{
    using webcrown::server::http::parse_phase;
    using webcrown::server::http::parser;

    // Expected
    const std::string expected_error_msg = "http incomplete start line";
    const parse_phase expected_parse_phase = parse_phase::parse_target;

    // Scenario
    char const* raw = "/user/12903 ";

    parser p{};
    std::error_code ec{};
    std::string_view target;

    char const*& it = raw;
    char const* last = raw + std::strlen(raw);

    p.parse_target(it, last, target, ec);

    // Assert
    ASSERT_EQ(expected_error_msg, ec.message());
    ASSERT_EQ(expected_parse_phase, p.parse_phase());
}

TEST(HTTP_PARSER, parse_target_with_two_spaces_after_should_return_expected_target)
{
    // This test tests the (increment) at the end of function

    using webcrown::server::http::parse_phase;
    using webcrown::server::http::parser;

    // Expected
    const std::string expected_target = "/user/12903";
    const parse_phase expected_parse_phase = parse_phase::parse_target_finished;

    // Scenario
    char const* raw = "/user/12903 HTTP/1.1";

    parser p{};
    std::error_code ec{};
    std::string_view target;

    char const*& it = raw;
    char const* last = raw + std::strlen(raw);

    p.parse_target(it, last, target, ec);

    // Assert
    ASSERT_EQ(expected_target, std::string(target));
    ASSERT_EQ(expected_parse_phase, p.parse_phase());
}