#pragma once
#include "webcrown/server/http/middlewares/http_middleware.hpp"
#include "webcrown/server/http/middlewares/route.hpp"
#include <functional>
#include <vector>
#include <memory>
#include <spdlog/logger.h>

namespace webcrown {
namespace server {
namespace http {

enum auth_authorization_level
{
    none,
    guest,
    owner
};

/**
 * Auth middleware
 * This class is authentication and authorization
 */
class auth_middleware : public middleware
{
public:
    using auth_callback = std::function<bool(
            std::string const& token,
            std::shared_ptr<route> route,
            auth_authorization_level level)>;

    explicit auth_middleware(std::shared_ptr<spdlog::logger> logger)
        : logger_(logger)
        , should_return_now_(false)
    {}

    auth_middleware(auth_middleware const&) = delete;
    auth_middleware(auth_middleware&&) = delete;

    auth_middleware& operator=(auth_middleware const&) = delete;
    auth_middleware& operator=(auth_middleware&&) = delete;

    void on_setup(http_request const &request, http_response &response) override
    {
        auto forbidden_error = [&response, this]()
        {
            response.set_status(http_status::forbidden);

            should_return_now_ = true;
        };

        try
        {
            for(auto const& r : routes_)
            {
                auto&& route = r.first;
                if(!(route->is_match_with_target_request(request.target()) &&
                     route->method() == request.method()))
                {
                    continue;
                }

                // Verify if header Authorization exists
                auto headers = request.headers();
                auto auth_header = headers.find("Authorization");
                if (auth_header == headers.end())
                {
                    forbidden_error();
                    return;
                }

                // extract the token
                auto token = extract_token(auth_header->second);

                if (token.empty())
                {
                    logger_->error("Token is empty");
                    forbidden_error();
                    return;
                }

                assert(cb_ != nullptr);
                // Verify token
                if(!cb_(token, route, r.second))
                {
                    response.set_status(http_status::unauthorized);

                    should_return_now_ = true;
                    return;
                }
            }
        }
        catch(std::exception const& ex)
        {
            logger_->error("[auth_middleware] Error. {}",
                    ex.what());

            response.set_status(http_status::internal_server_error);
            return;
        }
    }

    void authorize_route(std::shared_ptr<route>& route, auth_authorization_level level)
    {
        routes_.emplace_back(std::make_pair(route, level));
    }

    bool should_return_now() override
    {
        return should_return_now_;
    }

    void should_return_now(bool flag) override
    {
        should_return_now_ = flag;
    }

    auth_callback callback() const { return cb_; }
    void callback(auth_callback cb) { cb_ = cb; }

private:
    std::string extract_token(std::string const& header_value)
    {
        auto index = header_value.find("Bearer ");
        if (index == std::string::npos)
            return "";

        auto result = header_value.substr(index + 7);

        return result;
    }
private:
    std::shared_ptr<spdlog::logger> logger_;
    bool should_return_now_;
    std::vector<std::pair<std::shared_ptr<route>, auth_authorization_level>> routes_;
    auth_callback cb_;
};

} // namespace http
} // namespace server
} // namespace webcrown
