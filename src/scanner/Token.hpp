#ifndef POISE_TOKEN_HPP
#define POISE_TOKEN_HPP

#include "../Poise.hpp"
#include "TokenType.hpp"

#include <string_view>

namespace poise::scanner {
class Token
{
public:
    Token(TokenType tokenType, usize line, usize column, std::string_view text);

    [[nodiscard]] auto length() const noexcept -> usize;
    [[nodiscard]] auto text() const noexcept -> std::string_view;
    [[nodiscard]] auto string() const noexcept -> std::string;
    [[nodiscard]] auto tokenType() const noexcept -> TokenType;
    [[nodiscard]] auto line() const noexcept -> usize;
    [[nodiscard]] auto column() const noexcept -> usize;
    auto print() const -> void;

private:
    TokenType m_tokenType;
    usize m_line, m_column;
    std::string_view m_text;
    std::string m_errorMessage;
};  // class Token
}   // namespace poise::scanner

#endif  // #ifndef POISE_TOKEN_HPP
