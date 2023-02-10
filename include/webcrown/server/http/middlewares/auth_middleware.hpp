#pragma once
#include "webcrown/server/http/middlewares/http_middleware.hpp"
#include "webcrown/server/http/middlewares/route.hpp"
#include <functional>
#include <vector>
#include <memory>


namespace webcrown {
namespace server {
namespace http {

enum auth_authorization_level
{
    none,
    guest,
    owner
};

struct auth_result
{
    explicit auth_result(bool success, std::string const& reason)
        : success(success)
        , reason(reason)
    {}

    bool success{false};
    std::string reason;
};

/**
 * Auth middleware
 * This class is authentication and authorization
 */
class auth_middleware : public middleware
{
public:
    using auth_callback = std::function<auth_result(
            std::string const& token,
            std::shared_ptr<route> route,
            auth_authorization_level level)>;

    explicit auth_middleware()
        : should_return_now_(false)
    {}

    auth_middleware(auth_middleware const&) = delete;
    auth_middleware(auth_middleware&&) = delete;

    auth_middleware& operator=(auth_middleware const&) = delete;
    auth_middleware& operator=(auth_middleware&&) = delete;

    bool execute(http_request const &request, http_response &response, std::shared_ptr<spdlog::logger> logger) override
    {
        auto forbidden_error = [&response, this]()
        {
            response.set_status(http_status::forbidden);
        };

        try
        {
            for(auto const& r : routes_)
            {
                auto&& route = r.first;
                if(!(route->is_match_with_target_request(request.target(), request.method())))
                {
                    continue;
                }

                // Verify if header Authorization exists
                auto headers = request.headers();
                auto auth_header = headers.find("Authorization");
                if (auth_header == headers.end())
                {
                    auth_header = headers.find("authorization");
                    if (auth_header == headers.end())
                    {
                        forbidden_error();
                        return false;
                    }
                }

                // extract the token
                auto token = extract_token(auth_header->second);

                if (token.empty())
                {
                    //logger_->error("Token is empty");
                    forbidden_error();
                    return false;
                }

                assert(cb_ != nullptr);
                // Verify token
                auto result = cb_(token, route, r.second);
                if(!result.success)
                {
                    std::string body_res = R"({"error": ")";
                    body_res.append(result.reason);
                    body_res.append(R"("})");

                    response.set_body(body_res);
                    response.set_status(http_status::unauthorized);

                    return false;
                }
            }

            return true;
        }
        catch(std::exception const& ex)
        {
            SPDLOG_LOGGER_DEBUG(logger, "webcrown::auth_middleware::on_setup Error: {}",
                                ex.what());

            response.set_status(http_status::internal_server_error);
            return false;
        }
    }

    void authorize_route(std::shared_ptr<route>& route, auth_authorization_level level)
    {
        routes_.emplace_back(std::make_pair(route, level));
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
    std::vector<std::pair<std::shared_ptr<route>, auth_authorization_level>> routes_;
    auth_callback cb_;
};

} // namespace http
} // namespace server
} // namespace webcrown
