#ifndef POISE_TOKEN_HPP
#define POISE_TOKEN_HPP

#include "../poise.hpp"
#include "TokenType.hpp"

#include <cstdlib>
#include <string_view>

namespace poise::scanner
{
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

#endif
