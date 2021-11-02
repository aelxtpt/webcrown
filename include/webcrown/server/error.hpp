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

enum class service_error : uint8_t
{
    unknown = 0,
    service_not_started,
    service_already_started
};

enum class session_error : uint8_t
{
    unknown = 0,
    sent_bytes_is_zero,
    sent_buffer_is_nullptr
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

class service_error_category : public std::error_category
{
public:
    const char* name() const noexcept override
    {
        return "webcrown_service_error";
    }

    std::string message(int ec) const noexcept override
    {
        switch(static_cast<service_error>(ec))
        {
            case service_error::service_not_started:
                return "Service is not started";
            case service_error::service_already_started:
                return "Service is already started";
            default:
                return "Unknown service error";
        }
    }
};

class session_error_category : public std::error_category
{
public:
    const char* name() const noexcept override
    {
        return "webcrown_session_error";
    }

    std::string message(int ec) const noexcept override
    {
        switch(static_cast<session_error>(ec))
        {
            case session_error::sent_bytes_is_zero:
                return "You are sending zero bytes";
            default:
                return "Unknown error";
        }
    }
};

template<typename T>
asio::error_code
make_error(T ec)
{
	static server_error_category const cat{};
    return asio::error_code{static_cast<std::underlying_type_t<T>>(ec), cat};
}

} //server
} //webcrown
