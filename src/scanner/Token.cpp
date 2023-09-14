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
    }
}

namespace poise::scanner
{
    auto Token::length() const -> std::size_t
    {
        return m_text.length();
    }

    auto Token::tokenType() const -> TokenType
    {
        return m_tokenType;
    }

    auto Token::text() const -> std::string_view
    {
        return m_text;
    }

    auto Token::print() const -> void
    {
        fmt::print("Token {{ text = '{}', type = {} }}\n", m_text, m_tokenType);
    }
}
