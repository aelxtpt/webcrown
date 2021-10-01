#pragma once

#include <system_error>
#include <asio/error_code.hpp>

namespace webcrown {
namespace server {

enum class server_error : uint8_t
{
    unknown = 0,
	server_already_started,
    server_not_started
};

class server_error_category : public std::error_category
{
public:
	const char* name() const noexcept override
	{
		return "webcrown_server_error";
	}

	std::string message(int ec) const noexcept override
	{
        switch(static_cast<server_error>(ec))
		{
			case server_error::server_already_started:
				return "Server is already started";
			case server_error::server_not_started:
				return "Server is not started";
			default:
				return "Unknown server error";
        }
	}
};

inline
asio::error_code
make_error(server_error ec)
{
	static server_error_category const cat{};
	return asio::error_code{static_cast<std::underlying_type_t<server_error>>(ec), cat};
}

} //server
} //webcrown
