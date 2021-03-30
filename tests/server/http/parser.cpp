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

TEST(HTTP_PARSER, parse_protocol_filled_with_space_should_return_error)
{
    using webcrown::server::http::parse_phase;
    using webcrown::server::http::parser;

    // Expected
    const std::string expected_error_msg = "http bad version";
    const parse_phase expected_parse_phase = parse_phase::parse_protocol_version;

    // Scenario
    char const* raw = " HTTP/1.1";

    parser p{};
    std::error_code ec{};
    int protocol;

    char const*& it = raw;
    char const* last = raw + std::strlen(raw);

    p.parse_protocol(it, last, protocol, ec);

    // Assert
    ASSERT_EQ(expected_error_msg, ec.message());
    ASSERT_EQ(expected_parse_phase, p.parse_phase());
}

TEST(HTTP_PARSER, parse_protocol_incosistent_version_should_return_error)
{
    using webcrown::server::http::parse_phase;
    using webcrown::server::http::parser;

    // Expected
    const std::string expected_error_msg = "http bad version";
    const parse_phase expected_parse_phase = parse_phase::parse_protocol_version;

    // Scenario
    char const* raw = "XTTP/1.1";

    parser p{};
    std::error_code ec{};
    int protocol{};

    char const*& it = raw;
    char const* last = raw + std::strlen(raw);

    p.parse_protocol(it, last, protocol, ec);

    // Assert
    ASSERT_EQ(expected_error_msg, ec.message());
    ASSERT_EQ(expected_parse_phase, p.parse_phase());
}

TEST(HTTP_PARSER, parse_protocol_all_ok_should_return_expected_version)
{
    using webcrown::server::http::parse_phase;
    using webcrown::server::http::parser;

    // Expected
    const int expected_version = 11;
    const parse_phase expected_parse_phase = parse_phase::parse_protocol_version_finished;

    // Scenario
    char const* raw = "HTTP/1.1";

    parser p{};
    std::error_code ec{};
    int protocol{};

    char const*& it = raw;
    char const* last = raw + std::strlen(raw);

    p.parse_protocol(it, last, protocol, ec);

    // Assert
    ASSERT_EQ(expected_version, protocol);
    ASSERT_EQ(expected_parse_phase, p.parse_phase());
}

TEST(HTTP_PARSER, parse_message_header)
{
    using webcrown::server::http::parse_phase;
    using webcrown::server::http::parser;

    // Expected

    // Scenario
    char const* raw = "User-Agent: PostmanRuntime/7.26.10\r\nAccept: */*\r\nPostman-Token: 957ca241-fd52-44eb-915a-5c40c5c21bfc\r\nHost: localhost:8443\r\nAccept-Encoding: gzip, deflate, br\r\nConnection: keep-alive\r\n\r\n";

    parser p{};
    std::error_code ec{};

    char const*& it = raw;
    char const* last = raw + std::strlen(raw);

    std::unordered_map<std::string, std::string> headers{};

    p.parse_message_header(it, last, headers, ec);

    // Assert
    ASSERT_TRUE(headers.size() > 0);
    ASSERT_EQ(headers["User-Agent"], "PostmanRuntime/7.26.10");
    ASSERT_EQ(headers["Accept"], "*/*");
    ASSERT_EQ(headers["Postman-Token"], "957ca241-fd52-44eb-915a-5c40c5c21bfc");
    ASSERT_EQ(headers["Host"], "localhost:8443");
    ASSERT_EQ(headers["Accept-Encoding"], "gzip, deflate, br");
    ASSERT_EQ(headers["Connection"], "keep-alive");
}