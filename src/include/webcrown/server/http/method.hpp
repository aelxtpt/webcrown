#pragma once
#include <string_view>

// Faz sentido, cada enum ter um arquivo
// Qualquer tipo de operacao sobre esse enum, fica nesse arquivo
// Converters e etc
// Bem parecido com classe
// Acho que a organizacao fica legal, apesar de varios arquivos
// Mas pelo menos sabemos onde procurar cada coisa

namespace webcrown {
namespace server {
namespace http {

enum class method : std::uint8_t
{
    unknown = 0,

    post,
    get,
    delet
    // TODO: add other methods
};

/// Converts HTTP method enum to the HTTP method string
/// \param m method enum
/// \return HTTP method as string
inline
std::string_view
to_string(method m);

/// Converts a HTTP method string to the HTTP method enum
/// \param m string method
/// \return HTTP method as enum
inline
method
to_method(std::string_view m);

}}}

#include "method.inl"
