#pragma once
#include "webcrown/server/http/middlewares/http_middleware.hpp"
#include "webcrown/server/http/middlewares/route.hpp"
#include <functional>
#include <unordered_map>
#include <memory>
#include <spdlog/logger.h>

namespace webcrown {
namespace server {
namespace http {

class routing_middleware : public middleware {

    std::vector<std::shared_ptr<route>> routers_;
    std::shared_ptr<spdlog::logger> logger_;
public:
    explicit routing_middleware(std::shared_ptr<spdlog::logger> logger)
        : logger_(logger)
    {}

    routing_middleware(routing_middleware const &) = delete;
    routing_middleware(routing_middleware &&) = delete;

    routing_middleware &operator=(routing_middleware const &) = delete;
    routing_middleware &operator=(routing_middleware &&) = delete;

    void on_setup(http_request const &request, http_response &response) override
  {
	  bool route_found{false};

	  // find route
	  for (auto const &r : routers_)
	  {
	      logger_->info("[routing_middleware] Matching target any route with {}", request.target());

		  if (r->is_match_with_target_request(request.target()) && r->method() == request.method())
		  {
		      auto&& cb = r->callback();

		      try
              {
		          cb(request, response);
              }
		      catch(std::exception const& ex)
              {
		          logger_->error("[routing_middleware] Error on callback of the router. {}",
                           ex.what());

		          response.set_status(http_status::internal_server_error);
		          break;
              }

			  route_found = true;
			  break;
		  }
	  }
	
	  // if not found
	  // return http response 404
	  if (!route_found)
	  {
	      logger_->error("[routing_middleware] router not found!");
	      response.set_status(http_status::not_found);
	  }
  }

    void add_router(std::shared_ptr<route> const route)
    {
        routers_.push_back(route);
    }

    bool should_return_now() override
    {
        return false;
    }

    void should_return_now(bool flag) override
    {
        //
    }
};

} // namespace http
} // namespace server
} // namespace webcrown
