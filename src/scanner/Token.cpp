#include "Token.hpp"

#include <fmt/format.h>

#include <algorithm>
#include <vector>

namespace fmt
{
    using namespace poise::scanner;

    auto formatter<TokenType>::format(TokenType tokenType, format_context& context) const
    {
        switch (tokenType) {
            case TokenType::And:
                return formatter<string_view>::format("And", context);
            case TokenType::End:
                return formatter<string_view>::format("End", context);
            case TokenType::Final:
                return formatter<string_view>::format("Final", context);
            case TokenType::Func:
                return formatter<string_view>::format("Func", context);
            case TokenType::Or:
                return formatter<string_view>::format("Or", context);
            case TokenType::PrintLn:
                return formatter<string_view>::format("PrintLn", context);
            case TokenType::Var:
                return formatter<string_view>::format("Var", context);
            case TokenType::Ampersand:
                return formatter<string_view>::format("Ampersand", context);
            case TokenType::Caret:
                return formatter<string_view>::format("Caret", context);
            case TokenType::CloseParen:
                return formatter<string_view>::format("CloseParen", context);
            case TokenType::Colon:
                return formatter<string_view>::format("Colon", context);
            case TokenType::Equal:
                return formatter<string_view>::format("Equal", context);
            case TokenType::EqualEqual:
                return formatter<string_view>::format("EqualEqual", context);
            case TokenType::Exclamation:
                return formatter<string_view>::format("Exclamation", context);
            case TokenType::Greater:
                return formatter<string_view>::format("Greater", context);
            case TokenType::GreaterEqual:
                return formatter<string_view>::format("GreaterEqual", context);
            case TokenType::Less:
                return formatter<string_view>::format("Less", context);
            case TokenType::LessEqual:
                return formatter<string_view>::format("LessEqual", context);
            case TokenType::Modulus:
                return formatter<string_view>::format("Modulus", context);
            case TokenType::Minus:
                return formatter<string_view>::format("Subtraction", context);
            case TokenType::NotEqual:
                return formatter<string_view>::format("NotEqual", context);
            case TokenType::OpenParen:
                return formatter<string_view>::format("OpenParen", context);
            case TokenType::Pipe:
                return formatter<string_view>::format("Pipe", context);
            case TokenType::Plus:
                return formatter<string_view>::format("Addition", context);
            case TokenType::Semicolon:
                return formatter<string_view>::format("Semicolon", context);
            case TokenType::ShiftLeft:
                return formatter<string_view>::format("ShiftLeft", context);
            case TokenType::ShiftRight:
                return formatter<string_view>::format("ShiftRight", context);
            case TokenType::Slash:
                return formatter<string_view>::format("Slash", context);
            case TokenType::Star:
                return formatter<string_view>::format("Star", context);
            case TokenType::Tilde:
                return formatter<string_view>::format("Tilde", context);
            case TokenType::False:
                return formatter<string_view>::format("False", context);
            case TokenType::Float:
                return formatter<string_view>::format("Float", context);
            case TokenType::Identifier:
                return formatter<string_view>::format("Identifier", context);
            case TokenType::Int:
                return formatter<string_view>::format("Int", context);
            case TokenType::None:
                return formatter<string_view>::format("None", context);
            case TokenType::String:
                return formatter<string_view>::format("String", context);
            case TokenType::True:
                return formatter<string_view>::format("True", context);
            case TokenType::EndOfFile:
                return formatter<string_view>::format("EndOfFile", context);
            case TokenType::Error:
                return formatter<string_view>::format("Error", context);
            default:
                POISE_UNREACHABLE();
                return formatter<string_view>::format("unknown", context);
        }
    }
}

namespace poise::scanner
{
    auto isLiteral(TokenType tokenType) -> bool
    {
        return tokenType == TokenType::False
            || tokenType == TokenType::Float
            || tokenType == TokenType::Identifier
            || tokenType == TokenType::Int
            || tokenType == TokenType::None
            || tokenType == TokenType::String
            || tokenType == TokenType::True;
    }

    auto isUnaryOp(TokenType tokenType) -> bool
    {
        return tokenType == TokenType::Exclamation
            || tokenType == TokenType::Tilde
            || tokenType == TokenType::Minus
            || tokenType == TokenType::Plus;
    }

    Token::Token(TokenType tokenType, usize line, usize column, std::string_view text)
        : m_tokenType{tokenType}
        , m_line{line}
        , m_column{column}
        , m_text{text}
    {

    }

    auto Token::length() const -> usize
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

    auto Token::line() const -> usize
    {
        return m_line;
    }

    auto Token::column() const -> usize
    {
        return m_column;
    }

    auto Token::print() const -> void
    {
        fmt::print("Token {{ type = {}, line = {}, column = {}, length = {}, text = '{}' }}\n", m_tokenType, m_line, m_column, m_text.length(), m_text);
    }
}

