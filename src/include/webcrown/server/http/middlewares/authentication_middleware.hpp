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

// TODO: Read the official documentation and adjust implementation
// This is my own implementation, so probably is an error prone implementation
enum class authorization_level
{
    OwnerPage,
    GuestPage
};

class authentication_middleware : public middleware
{
    using verification_callback =
        std::function<bool(std::string const& token, std::shared_ptr<route> route, authorization_level level)>;

public:
    explicit authentication_middleware(std::shared_ptr<spdlog::logger> logger, verification_callback cb = nullptr)
        : should_return_now_(false)
        , logger_(logger)
        , cb_(cb)
    {}

    authentication_middleware(authentication_middleware const &) = delete;
    authentication_middleware(authentication_middleware &&) = delete;

    authentication_middleware &operator=(authentication_middleware const &) = delete;
    authentication_middleware &operator=(authentication_middleware &&) = delete;

    void on_setup(http_request const &request, http_response &response) override
    {
        auto forbidden_error = [&response, this]()
        {
            response.set_status(http_status::forbidden);

            should_return_now_ = true;
        };

        try
        {
            for(auto const& r : accesses_)
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

                auto x1 = auth_header->first;
                auto x2 = auth_header->second;
                // extract the token
                auto token = extract_token(auth_header->second);

                if (token.empty())
                {
                    logger_->error("Token is empty");
                    forbidden_error();
                    return;
                }

                // verify authorization level
                // Unauthorized access
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
            logger_->error("[authentication_middleware] Error. {}",
                     ex.what());

            response.set_status(http_status::internal_server_error);
            return;
        }
    }

    void authorize_route(std::shared_ptr<route>& route, authorization_level level)
    {
        accesses_.emplace_back(std::make_pair(route, level));
    }

    bool should_return_now() override
    {
        return should_return_now_;
    }

    void should_return_now(bool flag) override
    {
        should_return_now_ = flag;
    }

    verification_callback callback() const { return cb_; }
    void callback(verification_callback cb) { cb_ = cb; }

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
    bool should_return_now_;
    std::shared_ptr<spdlog::logger> logger_;
    std::vector<std::pair<std::shared_ptr<route>, authorization_level>> accesses_;
    verification_callback cb_;
};

} // namespace http
} // namespace server
} // namespace webcrown
