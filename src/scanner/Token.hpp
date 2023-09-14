#ifndef POISE_TOKEN_HPP
#define POISE_TOKEN_HPP

#include <fmt/core.h>

#include <cstdlib>
#include <string_view>

namespace poise::scanner
{
    enum class TokenType
    {
        // keywords
        End,
        Func,
        PrintLn,

        // symbols
        OpenParen,
        CloseParen,
        Colon,
        Semicolon,

        // value types
        Float,
        Identifier,
        Int,
        String,

        // other
        EndOfFile,
        Error,
    };

    class Token
    {
    public:
        Token(TokenType tokenType, std::size_t line, std::size_t column, std::string_view text);

        [[nodiscard]] auto length() const -> std::size_t;
        [[nodiscard]] auto text() const -> std::string_view;
        [[nodiscard]] auto string() const -> std::string;
        [[nodiscard]] auto tokenType() const -> TokenType;
        [[nodiscard]] auto line() const -> std::size_t;
        [[nodiscard]] auto column() const -> std::size_t;
        auto print() const -> void;

    private:
        TokenType m_tokenType;
        std::size_t m_line, m_column;
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
