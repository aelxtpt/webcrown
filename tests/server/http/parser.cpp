#include <gmock/gmock.h>
#include "webcrown/server/http/parser.hpp"
#include <memory>

TEST(XPTOX, xpp)
{
    const std::string http_request = R"(GET /v1/ab0c5383 HTTP/1.1
Host: api.mocki.io
)";

    using webcrown::server::http::parser;
    parser p{};

    std::error_code ec{};

    p.parse(http_request.c_str(), http_request.size(), ec);
}
