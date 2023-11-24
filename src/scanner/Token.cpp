#include "Token.hpp"

#include <fmt/format.h>

#include <algorithm>
#include <vector>

namespace fmt
{
    using namespace poise::scanner;

    auto formatter<TokenType>::format(TokenType tokenType, format_context& context) const
    {
        string_view result = "unknown";

        switch (tokenType) {
            case TokenType::And:
                result = "And";
                break;
            case TokenType::End:
                result = "End";
                break;
            case TokenType::Final:
                result = "Final";
                break;
            case TokenType::Func:
                result = "Func";
                break;
            case TokenType::Or:
                result = "Or";
                break;
            case TokenType::PrintLn:
                result = "PrintLn";
                break;
            case TokenType::Return:
                result = "Return";
                break;
            case TokenType::Var:
                result = "Var";
                break;
            case TokenType::Ampersand:
                result = "Ampersand";
                break;
            case TokenType::Caret:
                result = "Caret";
                break;
            case TokenType::CloseParen:
                result = "CloseParen";
                break;
            case TokenType::Colon:
                result = "Colon";
                break;
            case TokenType::Comma:
                result = "Comma";
                break;
            case TokenType::Equal:
                result = "Equal";
                break;
            case TokenType::EqualEqual:
                result = "EqualEqual";
                break;
            case TokenType::Exclamation:
                result = "Exclamation";
                break;
            case TokenType::Greater:
                result = "Greater";
                break;
            case TokenType::GreaterEqual:
                result = "GreaterEqual";
                break;
            case TokenType::Less:
                result = "Less";
                break;
            case TokenType::LessEqual:
                result = "LessEqual";
                break;
            case TokenType::Modulus:
                result = "Modulus";
                break;
            case TokenType::Minus:
                result = "Subtraction";
                break;
            case TokenType::NotEqual:
                result = "NotEqual";
                break;
            case TokenType::OpenParen:
                result = "OpenParen";
                break;
            case TokenType::Pipe:
                result = "Pipe";
                break;
            case TokenType::Plus:
                result = "Addition";
                break;
            case TokenType::Semicolon:
                result = "Semicolon";
                break;
            case TokenType::ShiftLeft:
                result = "ShiftLeft";
                break;
            case TokenType::ShiftRight:
                result = "ShiftRight";
                break;
            case TokenType::Slash:
                result = "Slash";
                break;
            case TokenType::Star:
                result = "Star";
                break;
            case TokenType::Tilde:
                result = "Tilde";
                break;
            case TokenType::False:
                result = "False";
                break;
            case TokenType::Float:
                result = "Float";
                break;
            case TokenType::Identifier:
                result = "Identifier";
                break;
            case TokenType::Int:
                result = "Int";
                break;
            case TokenType::None:
                result = "None";
                break;
            case TokenType::String:
                result = "String";
                break;
            case TokenType::True:
                result = "True";
                break;
            case TokenType::EndOfFile:
                result = "EndOfFile";
                break;
            case TokenType::Error:
                result = "Error";
                break;
        }

        return formatter<string_view>::format(result, context);
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

