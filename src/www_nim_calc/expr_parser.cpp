#include "expr_parser.hpp"
#include "www_nim.hpp"

#include <cctype>
#include <list>
#include <utility>
#include <boost/multiprecision/cpp_int.hpp>

using boost::multiprecision::cpp_int;
using std::list;
using std::pair;
using namespace www_nim;

namespace expr_parser {

namespace {

// Recursive-descent parser/evaluator — see expr_parser.hpp for the grammar.
class parser {
public:
    explicit parser(const std::string& s) : s_(s), pos_(0) {}

    www parse_top() {
        www result = parse_expr();
        skip_ws();
        if (pos_ != s_.size()) {
            throw parse_error("unexpected trailing input at position " + std::to_string(pos_) +
                ": \"" + s_.substr(pos_) + "\"");
        }
        return result;
    }

private:
    const std::string& s_;
    size_t pos_;

    void skip_ws() {
        while (pos_ < s_.size() && std::isspace((unsigned char)s_[pos_])) pos_++;
    }

    // Consumes `tok` if it matches at the current position (after skipping whitespace),
    // returning whether it did.
    bool try_consume(const std::string& tok) {
        skip_ws();
        if (s_.compare(pos_, tok.size(), tok) == 0) {
            pos_ += tok.size();
            return true;
        }
        return false;
    }

    // Like try_consume("^"), but never matches when it's really the start of "^." (nim power) —
    // '^' alone (the ordinal exponent-of-w introducer, only ever tried directly after 'w') would
    // otherwise steal the '^' meant for an enclosing "^." operator, e.g. misparsing "w^.3" as
    // "w^" followed by a malformed exponent ".3" instead of "w", nim-power, "3".
    bool try_consume_ordinal_caret() {
        skip_ws();
        if (pos_ < s_.size() && s_[pos_] == '^' && !(pos_ + 1 < s_.size() && s_[pos_ + 1] == '.')) {
            pos_++;
            return true;
        }
        return false;
    }

    void expect(const std::string& tok) {
        if (!try_consume(tok)) {
            throw parse_error("expected \"" + tok + "\" at position " + std::to_string(pos_));
        }
    }

    cpp_int parse_uint() {
        skip_ws();
        size_t start = pos_;
        while (pos_ < s_.size() && std::isdigit((unsigned char)s_[pos_])) pos_++;
        if (pos_ == start) {
            throw parse_error("expected a number at position " + std::to_string(pos_));
        }
        return cpp_int(s_.substr(start, pos_ - start));
    }

    static uint256_t to_uint256(const cpp_int& v) {
        static const cpp_int limit = cpp_int(1) << 256;
        if (v < 0 || v >= limit) {
            throw parse_error("integer literal out of range (must fit in 256 bits)");
        }
        return (uint256_t)v;
    }

    static uint16_t to_uint16(const cpp_int& v) {
        if (v < 0 || v > 0xFFFF) {
            throw parse_error("integer literal out of range (must fit in 16 bits) for a w-exponent or coefficient below w^w");
        }
        return (uint16_t)v;
    }

    // wwterm := 'w' ('^' uint)? ('*' uint)? | uint
    // Only reachable through ww_exponent's parenthesized form — see there for why a bare
    // (unparenthesized) exponent can't be a full wwterm.
    ww parse_ww_term() {
        if (try_consume("w")) {
            uint16_t exponent = 1;
            if (try_consume("^")) {
                exponent = to_uint16(parse_uint());
            }
            uint16_t coefficient = 1;
            if (try_consume("*")) {
                coefficient = to_uint16(parse_uint());
            }
            return ww(list<pair<uint16_t, uint16_t>>{{exponent, coefficient}});
        }
        return ww(to_uint16(parse_uint()));
    }

    // wwexpr := wwterm ('+' wwterm)*
    ww parse_ww_expr() {
        ww result = parse_ww_term();
        while (try_consume("+")) {
            result = result + parse_ww_term();
        }
        return result;
    }

    // ww_exponent := 'w' | uint | '(' wwexpr ')'
    //
    // This is the exponent right after a top-level 'w^' (i.e. what goes into a www term's
    // ww-valued exponent slot). It's deliberately NOT the same as a bare wwterm/wwexpr: standard
    // exponentiation binds tighter than +/*, so "w^w*2" must mean "(w^w)*2" (a value just twice
    // w^w) and not "w^(w*2)" (an astronomically larger tower) — if a bare exponent greedily
    // consumed a trailing '*2' or '+...' the way a wwterm/wwexpr does, that '*2' would be stolen
    // from the outer expression and completely change the value's magnitude. Requiring explicit
    // parens for anything beyond a single 'w' or plain integer avoids that ambiguity entirely,
    // and matches www::string_of_term()'s own output exactly: it only omits parens around the
    // exponent when it's plain 'w' or a bare integer, and parenthesizes everything else.
    ww parse_ww_exponent() {
        if (try_consume("(")) {
            ww result = parse_ww_expr();
            expect(")");
            return result;
        }
        if (try_consume("w")) {
            return ww(list<pair<uint16_t, uint16_t>>{{1, 1}});
        }
        return ww(to_uint16(parse_uint()));
    }

    // atom := uint | 'w' ('^' ww_exponent)? | '(' expr ')'
    www parse_atom() {
        if (try_consume("(")) {
            www result = parse_expr();
            expect(")");
            return result;
        }
        if (try_consume("w")) {
            ww exponent = ww(1); // no '^E' given: w^1 == w
            if (try_consume_ordinal_caret()) {
                exponent = parse_ww_exponent();
            }
            return www(list<pair<ww, uint256_t>>{{exponent, 1}});
        }
        return www(to_uint256(parse_uint()));
    }

    // power := atom ('^.' uint)?
    www parse_power() {
        www base = parse_atom();
        if (try_consume("^.")) {
            cpp_int exponent = parse_uint();
            return www_nim_pow(base, exponent);
        }
        return base;
    }

    // term := power (('*'|'*.') power)*
    www parse_term() {
        www result = parse_power();
        while (true) {
            if (try_consume("*.")) {
                result = www_nim_mul(result, parse_power());
            } else if (try_consume("*")) {
                result = result * parse_power();
            } else {
                break;
            }
        }
        return result;
    }

    // expr := term (('+'|'+.') term)*
    www parse_expr() {
        www result = parse_term();
        while (true) {
            if (try_consume("+.")) {
                result = www_nim_add(result, parse_term());
            } else if (try_consume("+")) {
                result = result + parse_term();
            } else {
                break;
            }
        }
        return result;
    }
};

} // namespace

www parse_and_evaluate(const std::string& input) {
    parser p(input);
    return p.parse_top();
}

} // namespace expr_parser
