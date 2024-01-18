#include "Token.hpp"

#include <fmt/core.h>

namespace poise::scanner {
Token::Token(TokenType tokenType, usize line, usize column, std::string_view text)
    : m_tokenType{tokenType}
    , m_line{line}
    , m_column{column}
    , m_text{text}
{

}

auto Token::length() const noexcept -> usize
{
    return m_text.length();
}

auto Token::text() const noexcept -> std::string_view
{
    return m_text;
}

auto Token::string() const noexcept -> std::string
{
    return std::string{text()};
}

auto Token::tokenType() const noexcept -> TokenType
{
    return m_tokenType;
}

auto Token::line() const noexcept -> usize
{
    return m_line;
}

auto Token::column() const noexcept -> usize
{
    return m_column;
}

auto Token::print() const -> void
{
    fmt::print("Token {{ type = {}, line = {}, column = {}, length = {}, text = '{}' }}\n", m_tokenType, m_line, m_column, m_text.length(), m_text);
}
}   // namespace poise::scanner
 
