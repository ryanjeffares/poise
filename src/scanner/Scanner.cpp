#include "Scanner.hpp"

#include <cctype>
#include <fstream>
#include <sstream>

namespace poise::scanner
{
    Scanner::Scanner(const std::filesystem::path& inFilePath)
        : m_symbolLookup{
            {'&', TokenType::Ampersand},
            {'^', TokenType::Caret},
            {')', TokenType::CloseParen},
            {':', TokenType::Colon},
            {'!', TokenType::Exclamation},
            {'>', TokenType::Greater},
            {'<', TokenType::Less},
            {'-', TokenType::Minus},
            {'%', TokenType::Modulus},
            {'(', TokenType::OpenParen},
            {'|', TokenType::Pipe},
            {'+', TokenType::Plus},
            {';', TokenType::Semicolon},
            {'/', TokenType::Slash},
            {'*', TokenType::Star},
            {'~', TokenType::Tilde},
        }
        , m_keywordLookup{
            {"and", TokenType::And},
            {"end", TokenType::End},
            {"final", TokenType::Final},
            {"func", TokenType::Func},
            {"false", TokenType::False},
            {"none", TokenType::None},
            {"or", TokenType::Or},
            {"println", TokenType::PrintLn},
            {"true", TokenType::True},
            {"var", TokenType::Var},
        }

    {
        std::ifstream inFileStream{inFilePath};
        std::stringstream inCodeStream;
        inCodeStream << inFileStream.rdbuf();
        m_code = inCodeStream.str();
    }

    auto Scanner::getCodeAtLine(usize line) const -> std::string_view
    {
        auto current = 1zu;
        auto strIndex = 0zu;

        while (current < line) {
            if (strIndex > m_code.length()) {
                return "";
            }

            strIndex++;

            if (m_code[strIndex - 1zu] == '\n') {
                current++;
            }
        }

        auto pos = m_code.find('\n', strIndex);
        return std::string_view{m_code.data() + strIndex, pos - strIndex};
    }

    auto Scanner::getNumLines() const -> usize
    {
        auto count = 0zu;
        for (char i : m_code) {
            if (i == '\n') {
                count++;
            }
        }

        return count;
    }

    auto Scanner::scanToken() -> Token
    {
        skipWhitespace();
        m_start = m_current;

        if (auto current = advance()) {
            if (std::isalpha(*current) || *current == '_') {
                return identifier();
            }

            if (std::isdigit(*current)) {
                return number();
            }

            switch (*current) {
                case '!':
                    return multiCharSymbol({{'=', TokenType::NotEqual}}, TokenType::Equal);
                case '=':
                    return multiCharSymbol({{'=', TokenType::EqualEqual}}, TokenType::Equal);
                case '<':
                    return multiCharSymbol({{'<', TokenType::ShiftLeft}, {'=', TokenType::LessEqual}}, TokenType::Less);
                case '>':
                    return multiCharSymbol({{'>', TokenType::ShiftRight}, {'=', TokenType::GreaterEqual}}, TokenType::Greater);
                case '"':
                    return string();
                default: {
                    if (auto t = m_symbolLookup.find(*current); t != m_symbolLookup.end()) {
                        return makeToken(t->second);
                    }

                    return {TokenType::Error, m_line, m_column, "Invalid text"};
                }
            }
        }

        return {TokenType::EndOfFile, m_line, m_column, ""};
    }

    auto Scanner::skipWhitespace() -> void
    {
        while (auto c = peek()) {
            switch (*c) {
                case '\t':
                case '\r':
                case ' ':
                    advance();
                    break;
                case '\n':
                    m_line++;
                    m_column = 0zu;
                    advance();
                    break;
                case '/': {
                    if (peekNext().value_or(char{}) == '/') {
                        while (peek().value_or(char{}) != '\n') {
                            advance();
                        }
                    } else {
                        return;
                    }
                    break;
                }
                default:
                    return;
            }
        }
    }

    auto Scanner::advance() -> std::optional<char>
    {
        if (m_current >= m_code.length()) {
            return {};
        } else {
            m_current++;
            m_column++;
            return m_code[m_current - 1zu];
        }
    }

    auto Scanner::peek() -> std::optional<char>
    {
        if (m_current >= m_code.length()) {
            return {};
        } else {
            return m_code[m_current];
        }
    }

    auto Scanner::peekNext() -> std::optional<char>
    {
        if (m_current >= m_code.length() - 1zu) {
            return {};
        } else {
            return m_code[m_current + 1zu];
        }
    }

    auto Scanner::peekPrevious() -> std::optional<char>
    {
        if (m_current == 0zu) {
            return {};
        } else {
            return m_code[m_current - 1zu];
        }
    }

    auto Scanner::multiCharSymbol(const std::unordered_map<char, TokenType>& pairs, TokenType defaultType) -> Token
    {
        for (const auto [c, t] : pairs) {
            if (peek().value_or(char{}) == c) {
                advance();
                return makeToken(t);
            }
        }

        return makeToken(defaultType);
    }

    auto Scanner::identifier() -> Token
    {
        while (auto c = peek()) {
            if (std::isalnum(*c) || *c == '_') {
                advance();
            } else {
                break;
            }
        }

        if (auto t = m_keywordLookup.find(m_code.substr(m_start, m_current - m_start));
                t != m_keywordLookup.end()) {
            return makeToken(t->second);
        }

        return makeToken(TokenType::Identifier);
    }

    auto Scanner::number() -> Token
    {
        while (auto c = peek()) {
            if (std::isdigit(*c)) {
                advance();
            } else {
                break;
            }
        }

        if (peek().value_or(char{}) == '.' && std::isdigit(peekNext().value_or(char{}))) {
            advance();
            while (auto c = peek()) {
                if (std::isdigit(*c)) {
                    advance();
                } else {
                    break;
                }
            }

            return makeToken(TokenType::Float);
        }

        return makeToken(TokenType::Int);
    }

    auto Scanner::string() -> Token
    {
        while (true) {
            if (auto c = peek()) {
                if (*c == '"' && peekPrevious().value_or(char{}) != '\\') {
                    break;
                }

                if (*c == '\n') {
                    m_line++;
                    m_column = 0zu;
                }

                advance();
            } else {
                return {TokenType::Error, m_line, m_column, "Unterminated string"};
            }
        }

        advance();
        return makeToken(TokenType::String);
    }

    auto Scanner::makeToken(TokenType tokenType) -> Token
    {
        auto length = m_current - m_start;
        return {tokenType, m_line, m_column - length, std::string_view{m_code.data() + m_start, length}};
    }
}
