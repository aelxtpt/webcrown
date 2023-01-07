#pragma once
#include "webcrown/server/http/middlewares/http_middleware.hpp"
#include "webcrown/server/http/middlewares/route.hpp"
#include <functional>
#include <unordered_map>
#include <memory>


namespace webcrown {
namespace server {
namespace http {

class routing_middleware : public middleware {

    std::vector<std::shared_ptr<route>> routers_;
public:
    explicit routing_middleware()
    {}

    routing_middleware(routing_middleware const &) = delete;
    routing_middleware(routing_middleware &&) = delete;

    routing_middleware &operator=(routing_middleware const &) = delete;
    routing_middleware &operator=(routing_middleware &&) = delete;

    void on_setup(http_request const &request, http_response &response, std::shared_ptr<spdlog::logger> logger) override
  {
	  bool route_found{false};

      SPDLOG_LOGGER_DEBUG(logger, "webcrown::routing_middleware::on_setup");

	  // find route
	  for (auto const &r : routers_)
	  {
          SPDLOG_LOGGER_DEBUG(logger, "webcrown::routing_middleware::on_setup Matching any route with {}",
                       request.target());

		  if (r->is_match_with_target_request(request.target(), request.method()))
		  {
              SPDLOG_LOGGER_DEBUG(logger, "webcrown::routing_middleware::on_setup Route {} match!",
                                  r->uri_target());

		      auto&& cb = r->callback();

		      try
              {
		          cb(request, response, r->path_parameters());
              }
		      catch(std::exception const& ex)
              {
                  SPDLOG_LOGGER_DEBUG(logger, "webcrown::routing_middleware::on_setup Error on call callback of the router. {}",
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
          SPDLOG_LOGGER_DEBUG(logger, "webcrown::routing_middleware::on_setup route {} not found!",
                              request.target());
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
