#ifndef parser_h
#define parser_h

#include <cstdint>
#include <system_error>
#include <unordered_map>
#include <cstring>

#include "webcrown/server/http/method.hpp"
#include "enums.hpp"
#include "webcrown/server/http/error.hpp"
#include "webcrown/server/http/detail/parser.hpp"
#include "webcrown/server/http/http_request.hpp"
#include "webcrown/server/http/http_response.hpp"
#include "webcrown/server/http/method.hpp"

namespace webcrown {
namespace server {
namespace http {

static std::string_view make_string(char const* first, char const* last)
{
    return {first, static_cast<std::size_t>(last - first)};
}

//
// TODO: Se um individuo mal intencionado enviar um buffer muito grande
// esse buffer não deveria ser tratado no proprio socket ao inves de esperar
// chegar ao protocolo http pra capar a requisicao ?
//
//

// Structure of the http REQUEST message HTTP/1.1
//  Request       = Request-Line              ; Section 5.1
//                   *(( general-header        ; Section 4.5
//                    | request-header         ; Section 5.3
//                    | entity-header ) CRLF)  ; Section 7.1
//                   CRLF
//                    [ message-body ]          ; Section 4.3

/// At the moment, this parser is only for the HTTP ***REQUEST***
class parser
{
    parse_phase parse_phase_;

    // max header size
    std::uint32_t header_limit_ = 8192;

public:
    // TODO: Colocar os metodos como privado, mas precisa
    // de uma lib de reflection, a minha, especionar memoria ?
    // para testar os metodos privados

    /// startline is the first line of the http request buffer.
    /// The basic buffer of the request http is:
    ///     generic-message = start-line
    //                          *(message-header CRLF)
    //                          CRLF
    //                          [ message-body ]
    //      start-line      = Request-Line | Status-Line  (Status-line is for the response http message)
    /// \param buffer
    /// \param size
    /// \param ec
    std::optional<http_request> parse_start_line(const char* buffer, size_t size, std::error_code& ec);

    void parse_message_header(char const*& it, char const* last, std::unordered_map<std::string, std::string>& headers, std::error_code& ec);

    void parse_message_header_name(char const*& it, char const* last, std::string_view& header_name, std::error_code& ec);

    void parse_message_header_value(const char*& it, char const* last, std::string_view& header_value, std::error_code& ec);

    void parse_body(const char*& it, char const* last, std::string_view& body, std::error_code& ec);

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

    /// Extract the HTTP protocol version in the buffer
    /// \param it pointer to the current position in the buffer
    /// \param last end of the buffer
    /// \param protocol_version string_view result
    /// \param ec error result
    void parse_protocol(char const*& it, char const* last, int& protocol_version, std::error_code& ec);

    /// Returns the http parse phase
    /// \return parsephase enum
    parse_phase parsephase() const noexcept { return parse_phase_; }
};

inline
std::optional<http_request>
parser::parse_start_line(const char *buffer, size_t size, std::error_code& ec)
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
        return std::nullopt;

    std::string_view target;
    parse_target(it, last, target, ec);
    if (ec)
        return std::nullopt;

    int protocol_version{};
    parse_protocol(it, last, protocol_version, ec);
    if (ec)
        return std::nullopt;

    // we will only support http 1.1 at the moment
    if (protocol_version < 11 || protocol_version > 11)
    {
        ec = make_error(http_error::bad_version);
        return std::nullopt;
    }

    if(it + 2 > last)
    {
        ec = make_error(http_error::incomplete_start_line);
        return std::nullopt;
    }

    if(it[0] != '\r' && it[1] != '\n')
    {
        ec = make_error(http_error::invalid_request_line);
        return std::nullopt;
    }

    // Skip the CRLR
    it += 2;

    std::unordered_map<std::string, std::string> headers;
    parse_message_header(it, last, headers, ec);

    std::string_view body;
    parse_body(it, last, body, ec);

    http_request request(to_method(method), protocol_version, target, headers, body);
    return request;
}

inline
void
parser::parse_body(const char*& it, const char* last, std::string_view& body, std::error_code& ec)
{
    auto first = it;

    // check limit of length

    body = make_string(first, last);
}

inline
void
parser::parse_message_header(const char*& it, const char* last, std::unordered_map<std::string, std::string>& headers, std::error_code& ec)
{
    // Muito curta ?
    if(it + 2 > last)
    {
        ec = make_error(http_error::incomplete_message_header);
        return;
    }

    auto first = it;

    // Structure
    // message-header = field-name ":" [ field-value ]
    //       field-name     = token
    //       field-value    = *( field-content | LWS )
    //       field-content  = <the OCTETs making up the field-value
    //                        and consisting of either *TEXT or combinations
    //                        of token, separators, and quoted-string>

    // Parse fields
    for(;it < last; ++it)
    {
        std::string_view header_name{};
        std::string_view header_value{};

        parse_message_header_name(it, last, header_name, ec);

        parse_message_header_value(it, last, header_value, ec);

        // TODO: Optimize me, please
        std::string hn = std::string(header_name);
        std::string hv = std::string(header_value);
        headers.insert({hn, hv});

        // Final do header_name: header_value
        if(it[0] == '\r')
        {
            if(it[1] != '\n')
                ec = make_error(http_error::invalid_message_header_crlf);

            // end headers
            if (it[2] == '\r' && it[3] == '\n')
                break;

            // This will Skip crlf for complete, because the main for is already skip the \r
            ++it;
        }
    }
}

inline
void
parser::parse_message_header_name(const char*& it, const char* last, std::string_view& header_name,
                                  std::error_code& ec)
{
    auto first = it;

    for(;it < last; ++it)
    {
        if (*it == ':')
            break;

        if (!detail::is_valid_token(*it))
        {
            ec = make_error(http_error::bad_field);
            return;
        }
    }

    header_name = make_string(first, it++);

    //++it; // skip the : character
}

inline
void
parser::parse_message_header_value(const char*& it, const char* last, std::string_view& header_value,
                                   std::error_code& ec)
{
    // skip leading ' ' and '\t'
    for(;;++it)
    {
        if (it + 1 > last)
        {
            ec = make_error(http_error::incomplete_message_header);
            return;
        }
        // Deve começar com espaco ou tab
        if (! (*it == ' ' || *it == '\t'))
            break;
    }

    auto first = it;

    // Field value
    for(; it < last; ++it)
    {
        if(it[0] == '\r' && it[1] == '\n')
            break;

        // TODO: Can be undefined behavior
        // TODO: Refactor this part
        if (!std::isprint(*it))
        {
            ec = make_error(http_error::bad_field_value);
            return;
        }
    }

    header_value = make_string(first, it);
}

inline
void
parser::parse_method(char const*& it, char const* last, std::string_view& method, std::error_code& ec)
{
    parse_phase_ = parse_phase::parse_method;
    auto const first = it;

    if (std::isspace(*it))
    {
        ec = make_error(http_error::bad_method);
        return;
    }

    // Procura por method_token, que vai ser sempre seguido por um espaço
    for(; it < last; ++it)
    {
        // if is not an char
        if(!detail::is_token_char(*it))
            break;

        if (!std::isalpha(*it))
        {
            ec = make_error(http_error::bad_method);
            return;
        }
    }

    // Muito curta a string ?
    if (it + 1 >= last)
    {
        // Error: request line está incompleto
        ec = make_error(http_error::incomplete_start_line);
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
    method = make_string(first, it++);

    parse_phase_ = parse_phase::parse_method_finished;
}

inline
void
parser::parse_target(const char*& it, const char* last, std::string_view& target, std::error_code& ec)
{
    parse_phase_ = parse_phase::parse_target;
    auto const first = it;

    if (!detail::is_pathchar(*it))
    {
        ec = make_error(http_error::bad_target);
        return;
    }

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
    if (it + 1 >= last)
    {
        // Error: request line está incompleto
        ec = make_error(http_error::incomplete_start_line);
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
    target = make_string(first, it++);
    parse_phase_ = parse_phase::parse_target_finished;
}

inline
void
parser::parse_protocol(const char*& it, const char* last, int& protocol_version, std::error_code& ec)
{
    parse_phase_ = parse_phase::parse_protocol_version;

    // HTTP/1.1 <--- 8 characters
    if (it + 8 > last)
    {
        ec = make_error(http_error::incomplete_start_line);
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
    parse_phase_ = parse_phase::parse_protocol_version_finished;
}

}}}

#endif /* parser_h */
