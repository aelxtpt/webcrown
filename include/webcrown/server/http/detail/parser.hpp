#ifndef WEBCROWN_PARSER_HPP
#define WEBCROWN_PARSER_HPP

#include <assert.h>

namespace webcrown {
namespace server {
namespace http {
namespace detail {

// https://tools.ietf.org/html/rfc822#section-3
// https://tools.ietf.org/html/rfc7231
// https://tools.ietf.org/html/rfc5234#appendix-B.1

inline
char is_token_char(char c)
{
    /*
        tchar = "!" | "#" | "$" | "%" | "&" |
                "'" | "*" | "+" | "-" | "." |
                "^" | "_" | "`" | "|" | "~" |
                DIGIT | ALPHA
    */
    static char constexpr tab[] = {
            0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, // 0
            0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, // 16
            0, 1, 0, 1,  1, 1, 1, 1,  0, 0, 1, 1,  0, 1, 1, 0, // 32
            1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 0, 0,  0, 0, 0, 0, // 48
            0, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, // 64
            1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 0,  0, 0, 1, 1, // 80
            1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, // 96
            1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 0,  1, 0, 1, 0, // 112
            0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, // 128
            0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, // 144
            0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, // 160
            0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, // 176
            0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, // 192
            0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, // 208
            0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, // 224
            0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0  // 240
    };
    assert(sizeof(tab) == 256);
    return tab[static_cast<unsigned char>(c)];
}

inline
bool
is_pathchar(char c)
{
    // TEXT = <any OCTET except CTLs, and excluding LWS>
    static bool constexpr tab[256] = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //   0
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //  16
            0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //  32
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //  48
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //  64
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //  80
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //  96
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, // 112
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 128
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 144
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 160
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 176
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 192
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 208
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 224
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1  // 240
    };
    return tab[static_cast<unsigned char>(c)];
}

static
bool
is_digit(char c)
{
    return static_cast<unsigned char>(c-'0') < 10;
}

// field-name  =  1*<any CHAR, excluding CTLs, SPACE, and ":">
static
bool is_valid_token(char t)
{
    auto result = int(t);

    // Controls (CTLs) = %x00-1F / %x7F
    if((result >= 0x00 && result <= 0x1f) || result == 0x7f)
        return false;

    // Space
    if(result == 0x20)
        return false;
    // :
    if (result == 0x3A)
        return false;

    if (result >= 0x01 && result <= 0x7f)
        return true;

    return false;
};

// <any CHAR, including bare    ; => atoms, specials, comments and quoted-strings are NOT recognized.
//  CR & bare LF, but NOT       ;
//  including CRLF>
// Validation of CRLF out of function
static
bool is_valid_text(char t)
{
    auto result = int(t);

    // CHAR
    if (result >= 0x01 && result <= 0x7f)
        return true;

    return false;
}

//! Count of elements in static array
template <typename T, std::size_t N>
constexpr std::size_t
countof(const T (&)[N]) noexcept { return N; }

//! Count of elements in any other STL container
//template <typename T>
//std::size_t
//countof(const T& container) noexcept { return container.size(); }

}}}}

#endif //WEBCROWN_PARSER_HPP
