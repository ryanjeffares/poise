#include "Scanner.hpp"

#include <cctype>
#include <fstream>
#include <sstream>

namespace poise::scanner
{
    Scanner::Scanner(std::filesystem::path inFilePath)
        : m_symbolLookup{
            {'(', TokenType::OpenParen},
            {')', TokenType::CloseParen},
            {':', TokenType::Colon},
            {';', TokenType::Semicolon},
        }
        , m_keywordLookup{
            {"end", TokenType::End},
            {"func", TokenType::Func},
            {"println", TokenType::PrintLn},
        }

    {
        std::ifstream inFileStream{inFilePath};
        std::stringstream inCodeStream;
        inCodeStream << inFileStream.rdbuf();
        m_code = inCodeStream.str();
    }

    auto Scanner::getCodeAtLine(std::size_t line) const -> std::string_view
    {
        auto current = 1zu;
        auto strIndex = 0zu;

        while (current < line) {
            if (strIndex > m_code.length()) {
                return "";
            }

            strIndex++;

            if (m_code[strIndex - 1] == '\n') {
                current++;
            }
        }

        auto pos = m_code.find('\n', strIndex);
        return std::string_view{m_code.data() + strIndex, pos - strIndex};
    }

    auto Scanner::getNumLines() const -> std::size_t
    {
        auto count = 0zu;
        for (auto i = 0zu; i < m_code.length(); i++) {
            if (m_code[i] == '\n') {
                count++;
            }
        }

        return count;
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
                // TODO: comments
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

            if (auto t = m_symbolLookup.find(*current); t != m_symbolLookup.end()) {
                return makeToken(t->second);
            }

            if (*current == '"') {
                return string();
            }

            return Token(TokenType::Error, m_line, m_column, "Invalid text");
        }

        return Token(TokenType::EndOfFile, m_line, m_column, "");
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
                if (*c == '"') {
                    break;
                }

                if (*c == '\n') {
                    m_line++;
                    m_column = 0zu;
                }

                advance();
            } else {
                return Token(TokenType::Error, m_line, m_column, "Unterminated string");
            }
        }

        advance();
        return makeToken(TokenType::String);
    }

    auto Scanner::makeToken(TokenType tokenType) -> Token
    {
        auto length = m_current - m_start;
        return Token(tokenType, m_line, m_column - length, std::string_view{m_code.data() + m_start, length});
    }
}
