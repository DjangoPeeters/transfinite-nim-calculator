#ifndef EXPR_PARSER_HPP
#define EXPR_PARSER_HPP

#include "www.hpp"

#include <stdexcept>
#include <string>

namespace expr_parser {
    struct parse_error : std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    // Parses and evaluates an ordinal/nimber arithmetic expression, e.g. "w^3 + w*2" or
    // "w +. w" (nim-add). Grammar (see expr_parser.cpp for the full breakdown):
    //   expr        := term (('+'|'+.') term)*
    //   term        := power (('*'|'*.') power)*
    //   power       := atom ('^.' uint)?
    //   atom        := uint | 'w' ('^' ww_exponent)? | '(' expr ')'
    //   ww_exponent := 'w' | uint | '(' wwexpr ')'
    //   wwexpr      := wwterm ('+' wwterm)*
    //   wwterm      := 'w' ('^' uint)? ('*' uint)? | uint
    // '+'/'*' (and 'w^E') are ordinal (Cantor normal form) arithmetic — ww/www's own operators.
    // '+.'/'*.'/'^.' are nim (field) arithmetic — www_nim_add/mul/pow. A bare (unparenthesized)
    // exponent after 'w^' is restricted to just 'w' or a plain integer — exponentiation binds
    // tighter than +/*, so anything more (e.g. "w*2" or "w^2+3") needs explicit parens:
    // "w^(w*2)", "w^(w^2+3)" — matching exactly when www::to_string() itself omits/adds parens
    // around an exponent. Throws parse_error on any malformed input, including trailing garbage
    // after an otherwise-valid expression.
    www parse_and_evaluate(const std::string& input);
}

#endif
