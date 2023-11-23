#ifndef POISE_TOKEN_HPP
#define POISE_TOKEN_HPP

#include "../poise.hpp"

#include <fmt/core.h>

#include <cstdlib>
#include <string_view>

namespace poise::scanner
{
    enum class TokenType
    {
        // keywords
        And,
        End,
        Final,
        Func,
        Or,
        PrintLn,
        Var,

        // symbols
        Ampersand,
        Caret,
        CloseParen,
        Colon,
        Equal,
        EqualEqual,
        Exclamation,
        Greater,
        GreaterEqual,
        Less,
        LessEqual,
        Minus,
        Modulus,
        NotEqual,
        OpenParen,
        Pipe,
        Plus,
        Semicolon,
        ShiftLeft,
        ShiftRight,
        Slash,
        Star,
        Tilde,

        // value types
        False,
        Float,
        Identifier,
        Int,
        None,
        String,
        True,

        // other
        EndOfFile,
        Error,
    };

    [[nodiscard]] auto isLiteral(TokenType tokenType) -> bool;
    [[nodiscard]] auto isUnaryOp(TokenType tokenType) -> bool;

    class Token
    {
    public:
        Token(TokenType tokenType, usize line, usize column, std::string_view text);

        [[nodiscard]] auto length() const -> usize;
        [[nodiscard]] auto text() const -> std::string_view;
        [[nodiscard]] auto string() const -> std::string;
        [[nodiscard]] auto tokenType() const -> TokenType;
        [[nodiscard]] auto line() const -> usize;
        [[nodiscard]] auto column() const -> usize;
        auto print() const -> void;

    private:
        TokenType m_tokenType;
        usize m_line, m_column;
        std::string_view m_text;
    };
}

namespace fmt
{
    template<>
    struct formatter<poise::scanner::TokenType> : formatter<string_view>
    {
        auto format(poise::scanner::TokenType tokenType, format_context& context) const;
    };
}

#endif
