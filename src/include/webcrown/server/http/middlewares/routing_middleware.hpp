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
  routing_middleware() = default;

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
		  if (r->is_match_with_target_request(request.target()))
		  {
		      auto&& cb = r->callback();
			  cb(request, response);
			  route_found = true;
			  break;
		  }
	  }
	
	  // if not found
	  // return http response 404
	  if (!route_found)
	  {
		  // invoke http response 404
	  }
  }

  void add_router(std::shared_ptr<route> const route)
  {
      routers_.push_back(route);
  }
};

} // namespace http
} // namespace server
} // namespace webcrown
