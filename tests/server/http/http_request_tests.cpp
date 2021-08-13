#include <gmock/gmock.h>
#include "webcrown/server/http/http_parser.hpp"
#include "webcrown/server/http/detail/parser.hpp"
#include <memory>

//TEST(HTTP_REQUEST_TESTS, xpto)
//{
//    using webcrown::server::http::parse_phase;
//    using webcrown::server::http::parser;
//
//    // Expected
//
//    // Scenario
//    char const* raw_buffer = "GET /user/aelxtpt HTTP/1.1\r\nUser-Agent: PostmanRuntime/7.26.10\r\nAccept: */*\r\nPostman-Token: 942278c6-4a27-457a-84c5-cad490fcf41f\r\nHost: localhost:8443\r\nAccept-Encoding: gzip, deflate, br\r\nConnection: keep-alive\r\n\r\n";
//
//    std::error_code ec;
//    parser p;
//    auto http_request = p.parse_start_line(raw_buffer, std::strlen(raw_buffer), ec);
//
//    // Assert
//    ASSERT_TRUE(http_request);
//    http_request->target();
//}