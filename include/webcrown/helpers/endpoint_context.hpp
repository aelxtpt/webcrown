#pragma once

#include <utility>
#include <vector>
#include <memory>

#include <webcrown/server/http/middlewares/route.hpp>
#include <webcrown/server/http/middlewares/auth_middleware.hpp>
#include <webcrown/server/http/http_method.hpp>
#include <webcrown/server/http/http_request.hpp>
#include <webcrown/server/http/http_response.hpp>
#include <nlohmann/json.hpp>
#include <cassert>

namespace webcrown {
namespace helpers {

using std::vector;
using std::shared_ptr;
using std::make_shared;

using nlohmann::json;
using webcrown::server::http::route;
using webcrown::server::http::path_parameters_type;
using webcrown::server::http::http_method;
using webcrown::server::http::http_request;
using webcrown::server::http::http_context;
using webcrown::server::http::http_response;
using webcrown::server::http::http_status;
using webcrown::server::http::route_parameters_t;
using webcrown::server::http::auth_authorization_level;

class endpoint_context
{
    vector<shared_ptr<route>> routes_{};
    shared_ptr<webcrown::server::http::auth_middleware> _auth_middleware;
    decltype(_auth_middleware->callback()) _auth_callback;
public:
    endpoint_context()
    {
        _auth_middleware = make_shared<webcrown::server::http::auth_middleware>();
    }

    void add(shared_ptr<route> rt, bool require_authorize = false, auth_authorization_level auth_level = auth_authorization_level::none) 
    { 
        routes_.push_back(rt); 

        if(require_authorize)
            _auth_middleware->authorize_route(rt, auth_level);
    }

    decltype(routes_) routes() const { return routes_; }
    decltype(_auth_middleware) auth_middl() noexcept { return _auth_middleware; }
    bool should_add_auth_middleware() const noexcept 
    { 
        auto any = !(_auth_middleware->authorized_routes().empty());
        
        bool cb_is_defined = any && _auth_callback != nullptr;
        assert(cb_is_defined && "Route is marked as require authorization but callback is null");

        return any;
    }

    void configure_auth_callback(decltype(_auth_callback) cb)
    {
        _auth_callback = cb;
        _auth_middleware->callback(_auth_callback);
    }
};

inline
std::optional<std::string> 
json_any_missing_field()
{
    return std::nullopt;
}

inline
std::optional<std::string>
json_any_missing_field(json const&)
{
    return std::nullopt;
}

template<typename T = std::string, typename... Args>
std::optional<std::string>
json_any_missing_field(json const& request_json, T arg, Args... args)
{
    // TODO: if we pass string as request_json, it accept. Trait this
    if(!request_json.contains(arg))
        return arg;

    return json_any_missing_field(request_json, args...);
}


inline
std::optional<route_parameters_t>
get_parameter(std::vector<route_parameters_t> const& rt, std::string_view p_name)
{
    auto it = std::find_if(rt.cbegin(), rt.cend(),
                           [&p_name](route_parameters_t p)
    {
        if(p.name == p_name)
            return true;

        return false;
    });

    if(it == rt.cend())
        return std::nullopt;

    return *it;
}

inline
void
make_notfound_response(http_response& response, std::string_view msg)
{
    json response_body;
    response_body["error"] = msg;

    response.set_status(http_status::not_found);
    response.set_body(response_body.dump());
}

inline
void
make_response_json_error(http_response& response, http_status status, std::string_view msg)
{
    json response_body;
    response_body["error"] = msg;

    response.set_status(status);
    response.set_body(response_body.dump());
}

inline
void
make_badrequest_response(http_response& response, std::string_view msg)
{
    json response_body;
    response_body["error"] = msg;

    response.set_status(http_status::bad_request);
    response.set_body(response_body.dump());
}

inline
void
make_forbidden_response(http_response& response, std::string_view msg)
{
    json response_body;
    response_body["error"] = msg;

    response.set_status(http_status::forbidden);
    response.set_body(response_body.dump());
}

inline
void
make_internal_server_error_response(http_response& response, std::string_view msg)
{
    json response_body;
    response_body["error"] = msg;

    response.set_status(http_status::internal_server_error);
    response.set_body(response_body.dump());
}

}} // namespace webcrown::helpers