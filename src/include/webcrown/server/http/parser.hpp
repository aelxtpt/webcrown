#ifndef parser_h
#define parser_h

#include <cstdint>
#include <system_error>

#include "webcrown/server/http/method.hpp"
#include "webcrown/server/http/enums.hpp"
#include "webcrown/server/http/error.hpp"
#include "webcrown/server/http/detail/parser.hpp"

namespace webcrown {
namespace server {
namespace http {

//Request line
//method_token uri protocol_version CLRF



//
// TODO: Se um individuo mal intencionado enviar um buffer muito grande
// esse buffer não deveria ser tratado no proprio socket ao inves de esperar
// chegar ao protocolo http pra capar a requisicao ?
//
//


class parser
{
    parse_phase parse_phase_;

    // max header size
    std::uint32_t header_limit_ = 8192;

public:

    // will return an HTTP request ? Hm...
    void parse(const char* buffer, size_t size, std::error_code& ec);

    // parse request line
    void parse_request_line();

    /// Extract the HTTP method in the buffer
    /// \param it pointer to the first position on the buffer
    /// \param last end of the buffer
    /// \param method string_view result
    /// \param ec error result
    void parse_method(char const*& it, char const* last, std::string_view& method, std::error_code& ec); // Case sensitive

    /// Extract the HTTP target, aka URI in the buffer
    /// \param it pointer to the current position on the buffer
    /// \param last end of the buffer
    /// \param target string_view result
    /// \param ec error result
    void parse_target(char const*& it, char const* last, std::string_view& target, std::error_code& ec);

    void parse_protocol(char const*& it, char const* last, int& protocol_version, std::error_code& ec);
};

inline
void
parser::parse(const char *buffer, size_t size, std::error_code& ec)
{
    parse_phase_ = parse_phase::started;

    // Current position of the buffer in the parser
    char const*& it = buffer;

    // last character in the buffer
    char const* last = buffer + size;

    // request-line   = method SP request-target SP HTTP-version CRLF

    // Primeira coisa é procurar o methodo
    // Porque imagina, iterar todo o buffer pra achar o CRLF
    // vai que o buffer não tem o CRLR
    // Sendo que a primeira coisa que vem é o metodo seguido por um espaco
    // perdemos menos tempo
    std::string_view method;
    parse_method(it, last, method, ec);
    if (ec)
        return;

    std::string_view target;
    parse_target(it, last, target, ec);
    if (ec)
        return;

    int protocol_version{};
    parse_protocol(it, last, protocol_version, ec);
    if (ec)
        return;
}

static std::string_view make_string(char const* first, char const* last)
{
    return {first, static_cast<std::size_t>(last - first)};
}

inline
void
parser::parse_method(char const*& it, char const* last, std::string_view& method, std::error_code& ec)
{
    parse_phase_ = parse_phase::parse_method;
    auto const first = it;

    // Procura por method_token, que vai ser sempre seguido por um espaço
    for(; it < last; ++it)
    {
        // if is not an char
        if(!detail::is_token_char(*it))
            break;
    }

    // Muito curta a string ?
    if (it + 1 > last)
    {
        // Error: request line está incompleto
        ec = make_error(http_error::incomplete_request_line);
        return;
    }

    if(*it != ' ')
    {
        // It precisa ser o espaço após o token
        ec = make_error(http_error::bad_method);
        return;
    }

    if(it == last)
    {
        // cannot be empty
        ec = make_error(http_error::bad_method);
        return;
    }

    //++it is the SP (Single space)
    method = make_string(first, ++it);
}

inline
void
parser::parse_target(const char*& it, const char* last, std::string_view& target, std::error_code& ec)
{
    parse_phase_ = parse_phase::parse_target;
    auto const first = it;

    //Request line
    //method_token request- protocol_version CLRF
    // Procura pela uri
    for(; it < last; ++it)
    {
        //
        if(!detail::is_pathchar(*it))
            break;
    }

    // Muito curto a string ?
    if (it + 1 > last)
    {
        // Error: request line está incompleto
        ec = make_error(http_error::incomplete_request_line);
        return;
    }

    if(*it != ' ')
    {
        // It precisa ser o espaço após o request-target
        ec = make_error(http_error::bad_target);
        return;
    }

    if(it == last)
    {
        // cannot be empty
        ec = make_error(http_error::bad_target);
        return;
    }

    //++it is the SP (Single space)
    target = make_string(first, ++it);
}

inline
void
parser::parse_protocol(const char*& it, const char* last, int& protocol_version, std::error_code& ec)
{
    // HTTP/1.1 <--- 8 characters
    if (it + 8 > last)
    {
        ec = make_error(http_error::incomplete_request_line);
        return;
    }

    if (*it++ != 'H')
    {
        ec = make_error(http_error::bad_version);
        return;
    }
    if (*it++ != 'T')
    {
        ec = make_error(http_error::bad_version);
        return;
    }
    if (*it++ != 'T')
    {
        ec = make_error(http_error::bad_version);
        return;
    }
    if (*it++ != 'P')
    {
        ec = make_error(http_error::bad_version);
        return;
    }
    if (*it++ != '/')
    {
        ec = make_error(http_error::bad_version);
        return;
    }

    if (!detail::is_digit(*it))
    {
        ec = make_error(http_error::bad_version);
        return;
    }

    protocol_version = 10 * (*it++ - '0');

    if (*it++ != '.')
    {
        ec = make_error(http_error::bad_version);
        return;
    }

    if (!detail::is_digit(*it))
    {
        ec = make_error(http_error::bad_version);
        return;
    }

    protocol_version += *it++ - '0';
}

}}}

#endif /* parser_h */
