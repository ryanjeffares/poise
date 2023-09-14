#include "Token.hpp"

#include <fmt/format.h>

namespace fmt
{
    using namespace poise::scanner;

    auto formatter<TokenType>::format(TokenType tokenType, format_context& context) const
    {
        switch (tokenType) {
            case TokenType::End:
                return formatter<string_view>::format("End", context);
            case TokenType::Func:
                return formatter<string_view>::format("Func", context);
            case TokenType::PrintLn:
                return formatter<string_view>::format("PrintLn", context);
            case TokenType::OpenParen:
                return formatter<string_view>::format("OpenParen", context);
            case TokenType::CloseParen:
                return formatter<string_view>::format("CloseParen", context);
            case TokenType::Colon:
                return formatter<string_view>::format("Colon", context);
            case TokenType::Semicolon:
                return formatter<string_view>::format("Semicolon", context);
            case TokenType::Float:
                return formatter<string_view>::format("Float", context);
            case TokenType::Identifier:
                return formatter<string_view>::format("Identifier", context);
            case TokenType::Int:
                return formatter<string_view>::format("Int", context);
            case TokenType::String:
                return formatter<string_view>::format("String", context);
            case TokenType::EndOfFile:
                return formatter<string_view>::format("EndOfFile", context);
            case TokenType::Error:
                return formatter<string_view>::format("Error", context);
        }

        return formatter<string_view>::format("Unknown", context);
    }
}

namespace poise::scanner
{
    Token::Token(TokenType tokenType, std::size_t line, std::size_t column, std::string_view text)
        : m_tokenType{tokenType}
        , m_line{line}
        , m_column{column}
        , m_text{text}
    {

    }

    auto Token::length() const -> std::size_t
    {
        return m_text.length();
    }

    auto Token::text() const -> std::string_view
    {
        return m_text;
    }

    auto Token::string() const -> std::string
    {
        return std::string{text()};
    }

    auto Token::tokenType() const -> TokenType
    {
        return m_tokenType;
    }

    auto Token::line() const -> std::size_t
    {
        return m_line;
    }

    auto Token::column() const -> std::size_t
    {
        return m_column;
    }

    auto Token::print() const -> void
    {
        fmt::print("Token {{ type = {}, line = {}, column = {}, length = {}, text = '{}' }}\n", m_tokenType, m_line, m_column, m_text.length(), m_text);
    }
}
