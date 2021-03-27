#ifndef WEBCROWN_PARSER_HPP
#define WEBCROWN_PARSER_HPP

namespace webcrown {
namespace server {
namespace http {
namespace detail {

// TODO: Entender, na aula de standardford lá tem isso
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
