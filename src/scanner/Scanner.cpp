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
                    m_column = 0;
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
            return m_code[m_current - 1];
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
        if (m_current >= m_code.length() - 1) {
            return {};
        } else {
            return m_code[m_current + 1];
        }
    }

    auto Scanner::peekPrevious() -> std::optional<char>
    {
        if (m_current == 0) {
            return {};
        } else {
            return m_code[m_current - 1];
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

            return Token(TokenType::Error, "");
        }

        return Token(TokenType::EndOfFile, "");
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
                    m_column = 0;
                }

                advance();
            } else {
                return Token(TokenType::Error, "Unterminated string");
            }
        }

        advance();
        return makeToken(TokenType::String);
    }

    auto Scanner::makeToken(TokenType tokenType) -> Token
    {
        return Token(tokenType, std::string_view{m_code.data() + m_start, m_current - m_start});
    }
}
