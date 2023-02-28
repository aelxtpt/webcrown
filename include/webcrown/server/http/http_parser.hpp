#ifndef parser_h
#define parser_h

#include <cstdint>
#include <system_error>
#include <unordered_map>
#include <cstring>
#include <string>
#include <optional>

#include "webcrown/server/http/http_method.hpp"
#include "enums.hpp"
#include "webcrown/server/http/error.hpp"
#include "webcrown/server/http/detail/parser.hpp"
#include "webcrown/server/http/http_request.hpp"
#include "webcrown/server/http/http_response.hpp"
#include "webcrown/server/http/http_method.hpp"
#include <spdlog/spdlog.h>
#include <fstream>

#include <stdlib.h>


namespace webcrown {
namespace server {
namespace http {

static std::string_view make_string(char const* first, char const* last)
{
    return {first, static_cast<std::size_t>(last - first)};
}

enum class content_type
{
    text,
    image,
    audio,
    video,
    application_json,
    multipart_formdata,
    message,
    unknown,
    not_specified,
    image_jpeg
};

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
    std::string method_;
    std::string target_;
    int protocol_version_;
    std::unordered_map<std::string, std::string> headers_;
    std::string body_;
    std::vector<http_form_upload> uploads_;
    content_type header_content_type_;
    std::size_t buffer_size_readed_;
    std::shared_ptr<std::vector<std::byte>> multipart_buffer_;
    std::size_t boundary_value_length_;
    std::string boundary_value_;
    std::unordered_map<std::string, std::string> upload_body_headers_;
    std::shared_ptr<spdlog::logger> logger_;
public:
    explicit parser(std::shared_ptr<spdlog::logger> logger)
        : parse_phase_(parse_phase::not_started)
        , protocol_version_(0)
        , buffer_size_readed_(0)
        , boundary_value_length_(0)
        , logger_(logger)
    {
        multipart_buffer_ = std::make_shared<std::vector<std::byte>>();
    }

    // TODO: Colocar os metodos como privado, mas precisa
    // de uma lib de reflection, a minha, especionar memoria ?
    // para testar os metodos privados

    std::optional<http_request> parse(const char* buffer, size_t size, std::error_code& ec);

    /// startline is the first line of the http request buffer.
    /// The basic buffer of the request http is:
    ///     generic-message = start-line
    //                          *(message-header CRLF)
    //                          CRLF
    //                          [ message-body ]
    //      start-line      = Request-Line | Status-Line  (Status-line is for the response http message)
    void parse_start_line(char const*& it, char const* last, std::error_code& ec);

    void parse_message_header(char const*& it, char const* last, std::unordered_map<std::string, std::string>& headers, std::error_code& ec);

    void parse_message_header_name(char const*& it, char const* last, std::string_view& header_name, std::error_code& ec);

    void parse_message_header_value(const char*& it, char const* last, std::string_view& header_value, std::error_code& ec);

    std::optional<http_request> parse_body(const char*& it, char const* last, std::string_view& body, std::error_code& ec);

    /// Extract the HTTP get_method in the buffer
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

    void parse_content_type(content_type& content_type,
                            std::unordered_map<std::string, std::string> const& headers);

    // TODO: We need block the high buffers on server socket layer
    // https://stackoverflow.com/questions/49169538/curl-doesnt-send-entire-form-data-in-http-post-request
    void parse_media_type(char const*& it, char const* last,
            std::unordered_map<std::string,
            std::string> const& headers,
            std::vector<http_form_upload>& uploads,
            std::error_code& ec);

    /// Returns the http parse phase
    /// \return parsephase enum
    parse_phase parsephase() const noexcept { return parse_phase_; }
};

inline
std::optional<http_request>
parser::parse(const char* buffer, size_t size, std::error_code& ec)
{
    // Current position of the buffer in the parser
    char const*& it = buffer;

    // last character in the buffer
    char const* last = buffer + size;

    // We assume that the initial buffer always has the complete http start line
   if(parse_phase_ == parse_phase::not_started)
   {
        if (size == 0)
        {
            ec = make_error(http_error::need_more);
            return std::nullopt;
        }
        parse_phase_ = parse_phase::parse_start_line;
        parse_start_line(it, last, ec);
   }

    // corner case that we need refactor later
    // TODO: For media types, we need handle to ?
    if(parse_phase_ == parse_phase::parse_content_type_finished)
    {
        auto content_length_h = headers_.find("content-length");
        if (content_length_h != headers_.end())
        {
            auto content_length = std::atoi(content_length_h->second.c_str());
            auto no_body = it + 4 >= last;
            if (content_length > 0 && no_body)
            {
                SPDLOG_LOGGER_DEBUG(logger_,
                                    "webcrown::http_parser content_length is greather than zero but we not received the body.");
                parse_phase_ = parse_phase::parse_body_pending;

                return std::nullopt;
            }
        }
    }

   if(parse_phase_ == parse_phase::parse_content_type_finished || parse_phase_ == parse_phase::parse_body_pending)
   {
        if (header_content_type_ == content_type::text ||
                header_content_type_ == content_type::application_json ||
                header_content_type_ == content_type::not_specified)
        {
            std::string_view body;
            auto res = parse_body(it, last, body, ec);
            parse_phase_ = parse_phase::finished;

            return res;
        }
        else if(header_content_type_ == content_type::image ||
                header_content_type_ == content_type::image_jpeg ||
                header_content_type_ == content_type::multipart_formdata)
        {
            parse_media_type(it, last, headers_, uploads_, ec);
        }
    }

    if(parse_phase_ == parse_phase::parse_media_type_need_more)
    {
        parse_media_type(it, last, headers_, uploads_, ec);
    }

    if (parse_phase_ == parse_phase::parse_media_type_finished)
    {
        parse_phase_ = parse_phase::finished;
        http_request request(to_method(method_), protocol_version_, target_, headers_, uploads_);
        return request;
    }


    return std::nullopt;
}

inline
void
parser::parse_start_line(char const*& it, char const* last, std::error_code& ec)
{
    parse_phase_ = parse_phase::parse_start_line_started;

    // request-line   = get_method SP request-target SP HTTP-version CRLF


    std::string_view method;

    parse_method(it, last, method, ec);
    if (ec)
        return;
    method_ = std::move(method);

    std::string_view target;
    parse_target(it, last, target, ec);
    if (ec)
        return;
    target_ = std::move(target);

    parse_protocol(it, last, protocol_version_, ec);
    if (ec)
        return;

    // we will only support http 1.1 at the moment
    if (protocol_version_ < 11 || protocol_version_ > 11)
    {
        ec = make_error(http_error::bad_version);
        return;
    }

    if(it + 2 > last)
    {
        ec = make_error(http_error::incomplete_start_line);
        return;
    }

    if(it[0] != '\r' && it[1] != '\n')
    {
        ec = make_error(http_error::invalid_request_line);
        return;
    }

    // Skip the CRLR
    it += 2;

    parse_message_header(it, last, headers_, ec);

    // Parse Content Type
    parse_content_type(header_content_type_, headers_);

    // TODO: At the moment, we will reject all unsuported content types
    // Supported: multipart and json
    if (header_content_type_ != content_type::application_json &&
        header_content_type_ != content_type::multipart_formdata &&
        header_content_type_ != content_type::not_specified &&
        header_content_type_ != content_type::image_jpeg)
    {
        ec = make_error(http_error::content_type_not_implemented);
        SPDLOG_LOGGER_DEBUG(logger_, "webcronw::http_parser Content-type is not implemented");
        return;
    }

//    if(header_content_type == content_type::multipart_formdata)
//    {
//        // Parse body
//

//        //http_request request(to_method(method), protocol_version, target, headers, uploads);
//        return;
//    }


//    parse_body(it, last, body, ec);

//    http_request request(to_method(method), protocol_version, target, headers, body);
//    return;
}

inline
std::optional<http_request>
parser::parse_body(const char*& it, const char* last, std::string_view& body, std::error_code& ec)
{
    auto first = it;

    // check limit of length

    body = make_string(first, last);

    http_request request(to_method(method_), protocol_version_, target_, headers_, body);
    return request;
}

inline
void
parser::parse_content_type(content_type& content_type,
                   std::unordered_map<std::string, std::string> const& headers)
{
    parse_phase_ = parse_phase::parse_content_type;

    auto content_type_h = headers.find("content-type");
    if (content_type_h == headers.end())
    {
        content_type = content_type::not_specified;
        parse_phase_ = parse_phase::parse_content_type_finished;
        return;
    }

    auto tokens = common::string_utils::split(content_type_h->second, ';');
    auto type = tokens.size() > 0 ? tokens[0] : content_type_h->second;

    // TODO: form-data is a subtype
    if (type == "multipart/form-data")
        content_type = content_type::multipart_formdata;
    else if(type == "application/json")
        content_type = content_type::application_json;
    else if(type == "image/jpeg")
        content_type = content_type::image_jpeg;
    else
        content_type = content_type::unknown;

    SPDLOG_LOGGER_DEBUG(logger_, "webcrown::http_parser content_type is: {}", type);

    parse_phase_ = parse_phase::parse_content_type_finished;
}

inline
void
parser::parse_media_type(
        char const*& it,
        char const* last,
        std::unordered_map<std::string, std::string> const& headers,
        std::vector<http_form_upload>& uploads,
        std::error_code& ec)
{
    auto verify_boundary_value = [&it, &last](std::size_t boundary_value_length, std::string const& bound_value) -> bool
    {
        // TODO: POOR
        for(; it < last; ++it)
        {
            auto match_count = 0;
            for(auto c = 0; c < boundary_value_length; ++c)
            {
                // linear scan for boundary value
                if (it[c] != bound_value[c])
                {
                    match_count = 0;
                    continue;
                }

                if (*it == '-')
                    continue;

                match_count++;
            }

            if (match_count == boundary_value_length)
            {
                return true;
            }

            //printf("%c", *it);
        }
        return false;
    };

    auto parse_media_type_begin = [this, &headers, &ec, &verify_boundary_value]
            (char const*& it, char const* last) -> void
    {
        parse_phase_ = parse_phase::parse_media_type;

        SPDLOG_LOGGER_DEBUG(logger_,
                            "webcrown::http_parser parsing media type.");

        auto content_type_h = headers.find("content-type");
        if (content_type_h == headers.end())
        {
            SPDLOG_LOGGER_DEBUG(logger_,
                                "webcrown::http_parser Content-Type header not found.");
            return;
        }

        // parses a media type value and any optional
        // parameters, per RFC 1521. Media types are the values in
        // Content-Type and Content-Disposition headers (RFC 2183).
        // On success, parse_media_type returns the media type converted
        // to lowercase and trimmed of white space

        // text
        // image
        // audio
        // video
        // application
        // multipart
        // message

        // The only header fields that have defined meaning for body parts are
        // those the namesof which begin with "Content-"

        // get boundary parameter
        auto boundary_pos = content_type_h->second.find("boundary=");
        if (boundary_pos == std::string::npos)
        {
            SPDLOG_LOGGER_DEBUG(logger_,
                                "webcrown::http_parser boundary not found.");
            ec = make_error(http_error::no_boundary_header_for_multipart);
            return;
        }

        boundary_value_ = content_type_h->second.substr(
                    boundary_pos, content_type_h->second.size());

        auto boundary_v_it = boundary_value_.begin();

        if (*boundary_v_it++ != 'b')
        {
            SPDLOG_LOGGER_DEBUG(logger_,
                                "webcrown::http_parser no boundary value exists.");
            return;
        }
        if (*boundary_v_it++ != 'o')
        {
            SPDLOG_LOGGER_DEBUG(logger_,
                                "webcrown::http_parser no boundary value exists.");
            return;
        }
        if (*boundary_v_it++ != 'u')
        {
            SPDLOG_LOGGER_DEBUG(logger_,
                                "webcrown::http_parser no boundary value exists.");
            return;
        }
        if (*boundary_v_it++ != 'n')
        {
            SPDLOG_LOGGER_DEBUG(logger_,
                                "webcrown::http_parser no boundary value exists.");
            return;
        }
        if (*boundary_v_it++ != 'd')
        {
            SPDLOG_LOGGER_DEBUG(logger_,
                                "webcrown::http_parser no boundary value exists.");
            return;
        }
        if (*boundary_v_it++ != 'a')
        {
            SPDLOG_LOGGER_DEBUG(logger_,
                                "webcrown::http_parser no boundary value exists.");
            return;
        }
        if (*boundary_v_it++ != 'r')
        {
            SPDLOG_LOGGER_DEBUG(logger_,
                                "webcrown::http_parser no boundary value exists.");
            return;
        }
        if (*boundary_v_it++ != 'y')
        {
            SPDLOG_LOGGER_DEBUG(logger_,
                                "webcrown::http_parser no boundary value exists.");
            return;
        }

        for(; boundary_v_it < boundary_value_.end(); ++boundary_v_it)
        {
            if (*boundary_v_it == '=' || *boundary_v_it == '-')
                continue;

            break;
        }

        boundary_value_ = std::string(make_string(&*boundary_v_it, &*boundary_value_.end()));

        // Consume CRLF
        if (it + 4 > last)
        {
            SPDLOG_LOGGER_DEBUG(logger_,
                                "webcrown::http_parser No body for multipart");
            return;
        }

        if (it[0] == '\r' &&
            it[1] == '\n' &&
            it[2] == '\r' &&
            it[3] == '\n')
        {
            it += 4;
        }

        // TODO: Loop to get multiple files

        boundary_value_length_ = boundary_value_.length();

        auto first = it;

        for(; it < last; ++it)
        {
            // find delimiter line
            if (*it == '-')
                continue;

            break;
        }

        verify_boundary_value(boundary_value_length_, boundary_value_);

        first = it;

        for(; it < last; ++it)
        {
            // At the end of the last boundary value has two --

            if(it[0] == '-' &&
               it[1] == '-' &&
               it[2] == '\r' &&
               it[3] == '\n')
            {
                SPDLOG_LOGGER_DEBUG(logger_,
                                    "webcrown::http_parser The epilogue boundary was reached.");
                return;
            }

            if(it[0] == '\r' &&
               it[1] == '\n')
            {
                break;
            }
        }

        auto boundary_value_at_line = std::string(make_string(first, it));

        if (boundary_value_at_line != boundary_value_)
        {
            SPDLOG_LOGGER_DEBUG(logger_,
                                "webcrown::http_parser Error on compare boundary value");
            return;
        }

        // CRLF
        it += 2;

        // Parse headers
        std::unordered_map<std::string, std::string> body_headers;
        parse_message_header(it, last, body_headers, ec);

        // parse headers body
        auto content_type_h_body = body_headers.find("content-type");
        if (content_type_h_body == body_headers.end())
        {
            SPDLOG_LOGGER_DEBUG(logger_,
                                "webcrown::http_parser No content-type on body");
            return;
        }

        upload_body_headers_ = body_headers;
        //img_type_ = content_type_h_body->second;

        // Consume Two CRLF
        it += 4;
    };

    auto parse_media_type_more = [this, &verify_boundary_value, &headers, &uploads]
            (char const*& it, char const* last) -> void
    {
        auto buffer_start = it;
        const char* buffer_end = last;

        auto content_length_h = headers.find("content-length");
        if (content_length_h == headers.end())
        {
            SPDLOG_LOGGER_DEBUG(logger_,
                                "webcrown::http_parser Content-length not found");
            return;
        }

        if (buffer_start == buffer_end)
        {
            // need more
            parse_phase_ = parse_phase::parse_media_type_need_more;
            return;
        }

        bool endbuffer_was_reached = false;

        auto first = it;
        if(verify_boundary_value(boundary_value_length_, boundary_value_))
        {
            for(;it < last; ++it)
            {
                if (*it == '-')
                    continue;

                if (it + 2 > last)
                {
                    SPDLOG_LOGGER_DEBUG(logger_,
                                        "webcrown::http_parser End of stream");
                    continue;
                }

                //printf("%c - %c\n", it[0], it[1]);

                if(it[0] == '\r' && it[1] == '\n')
                {
                    // consume \r and point to the end byte of the image
                    it += 2;
                    endbuffer_was_reached = true;
                    break;
                }
            }

            buffer_end = &*it;
        }
        else
            buffer_end = &*last;

        auto buffer_delta = buffer_end - buffer_start;
        buffer_size_readed_ += buffer_delta;

        // TODO: Optimize this
        multipart_buffer_->reserve(buffer_size_readed_);

        for(auto it = buffer_start; it < buffer_end; ++it)
        {
            auto x = (unsigned char*)it;
            unsigned char xx = *x;
            multipart_buffer_->push_back(std::byte{xx});
        }

        auto content_length = std::atoi(content_length_h->second.c_str());
        if (!endbuffer_was_reached && buffer_size_readed_ < content_length)
        {
            parse_phase_ = parse_phase::parse_media_type_need_more;
            return;
        }

        http_form_upload item;
        item.headers = upload_body_headers_;
        item.bytes = multipart_buffer_;
        uploads.push_back(std::move(item));

        parse_phase_ = parse_phase::parse_media_type_finished;
    };

    if (parse_phase_ == parse_phase::parse_content_type_finished)
    {
        parse_media_type_begin(it, last);

        // try read more
        parse_media_type_more(it, last);
    }
    else if (parse_phase_ == parse_phase::parse_media_type_need_more)
    {
        parse_media_type_more(it, last);
    }
}

inline
void
parser::parse_message_header(const char*& it, const char* last, std::unordered_map<std::string, std::string>& headers, std::error_code& ec)
{
    parse_phase_ = parse_phase::parse_headers;
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
        std::string hn = common::string_utils::to_lower(header_name);
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

    parse_phase_ = parse_phase::parse_headers_finished;
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
