#include "Scanner.hpp"

#include <cctype>
#include <fstream>
#include <sstream>

namespace poise::scanner {
Scanner::Scanner(const std::filesystem::path& inFilePath)
    : m_symbolLookup{
        {'&', TokenType::Ampersand},
        {'^', TokenType::Caret},
        {'}', TokenType::CloseBrace},
        {')', TokenType::CloseParen},
        {']', TokenType::CloseSquareBracket},
        {',', TokenType::Comma},
        {'.', TokenType::Dot},
        {'!', TokenType::Exclamation},
        {'>', TokenType::Greater},
        {'<', TokenType::Less},
        {'-', TokenType::Minus},
        {'%', TokenType::Modulus},
        {'{', TokenType::OpenBrace},
        {'(', TokenType::OpenParen},
        {'[', TokenType::OpenSquareBracket},
        {'|', TokenType::Pipe},
        {'+', TokenType::Plus},
        {';', TokenType::Semicolon},
        {'/', TokenType::Slash},
        {'*', TokenType::Star},
        {'~', TokenType::Tilde},
    }
    , m_keywordLookup{
        {"and", TokenType::And},
        {"as", TokenType::As},
        {"catch", TokenType::Catch},
        {"else", TokenType::Else},
        {"eprint", TokenType::EPrint},
        {"eprintln", TokenType::EPrintLn},
        {"export", TokenType::Export},
        {"false", TokenType::False},
        {"final", TokenType::Final},
        {"for", TokenType::For},
        {"func", TokenType::Func},
        {"if", TokenType::If},
        {"import", TokenType::Import},
        {"in", TokenType::In},
        {"none", TokenType::None},
        {"or", TokenType::Or},
        {"print", TokenType::Print},
        {"println", TokenType::PrintLn},
        {"return", TokenType::Return},
        {"this", TokenType::This},
        {"true", TokenType::True},
        {"try", TokenType::Try},
        {"typeof", TokenType::TypeOf},
        {"var", TokenType::Var},
        {"while", TokenType::While},
        {"Bool", TokenType::BoolIdent},
        {"Float", TokenType::FloatIdent},
        {"Int", TokenType::IntIdent},
        {"None", TokenType::NoneIdent},
        {"String", TokenType::StringIdent},
        {"Function", TokenType::FunctionIdent},
        {"Exception", TokenType::ExceptionIdent},
        {"List", TokenType::ListIdent},
    }
{
    std::ifstream inFileStream{inFilePath};
    std::stringstream inCodeStream;
    inCodeStream << inFileStream.rdbuf();
    m_code = inCodeStream.str();

    if (!m_code.empty() && m_code.back() != '\n') {
        // hack to make our compiler errors not throw assertion failures in fmt
        // if the error is on the last line of the file
        // and there's no trailing newline
        m_code.push_back('\n');
    }
}

auto Scanner::getCodeAtLine(usize line) const -> std::string_view
{
    auto current = 1_uz;
    auto strIndex = 0_uz;

    while (current < line) {
        if (strIndex > m_code.length()) {
            return "";
        }

        strIndex++;

        if (m_code[strIndex - 1_uz] == '\n') {
            current++;
        }
    }

    auto pos = m_code.find('\n', strIndex);
    return std::string_view{m_code.data() + strIndex, pos - strIndex};
}

auto Scanner::getNumLines() const noexcept  -> usize
{
    auto count = 0_uz;
    for (const auto i : m_code) {
        if (i == '\n') {
            count++;
        }
    }

    return count;
}

auto Scanner::scanToken() noexcept -> Token
{
    skipWhitespace();
    m_start = m_current;

    if (const auto current = advance()) {
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
            case ':': {
                if (peek().value_or(char{}) == ':') {
                    advance();
                    return makeToken(TokenType::ColonColon);
                } else {
                    return {TokenType::Error, m_line, m_column, std::string_view{m_code.data() + m_start, m_current - m_start}};
                }
            }
            default: {
                if (const auto t = m_symbolLookup.find(*current); t != m_symbolLookup.end()) {
                    return makeToken(t->second);
                }

                return {TokenType::Error, m_line, m_column, std::string_view{m_code.data() + m_start, m_current - m_start}};
            }
        }
    }

    return {TokenType::EndOfFile, m_line, m_column, ""};
}

auto Scanner::skipWhitespace() noexcept -> void
{
    while (const auto c = peek()) {
        switch (*c) {
            case '\t':
            case '\r':
            case ' ':
                advance();
                break;
            case '\n':
                m_line++;
                m_column = 0_uz;
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

auto Scanner::advance() noexcept -> std::optional<char>
{
    if (m_current >= m_code.length()) {
        return {};
    } else {
        m_current++;
        m_column++;
        return m_code[m_current - 1_uz];
    }
}

auto Scanner::peek() noexcept -> std::optional<char>
{
    if (m_current >= m_code.length()) {
        return {};
    } else {
        return m_code[m_current];
    }
}

auto Scanner::peekNext() noexcept -> std::optional<char>
{
    if (m_current >= m_code.length() - 1_uz) {
        return {};
    } else {
        return m_code[m_current + 1_uz];
    }
}

auto Scanner::peekPrevious() noexcept -> std::optional<char>
{
    if (m_current == 0_uz) {
        return {};
    } else {
        return m_code[m_current - 1_uz];
    }
}

auto Scanner::multiCharSymbol(const std::unordered_map<char, TokenType>& pairs, TokenType defaultType) noexcept -> Token
{
    for (const auto& [c, t] : pairs) {
        if (peek().value_or(char{}) == c) {
            advance();
            return makeToken(t);
        }
    }

    return makeToken(defaultType);
}

auto Scanner::identifier() noexcept -> Token
{
    while (const auto c = peek()) {
        if (std::isalnum(*c) || *c == '_') {
            advance();
        } else {
            break;
        }
    }

    if (const auto t = m_keywordLookup.find(m_code.substr(m_start, m_current - m_start));
        t != m_keywordLookup.end()) {
        return makeToken(t->second);
    }

    return makeToken(TokenType::Identifier);
}

auto Scanner::number() noexcept -> Token
{
    while (const auto c = peek()) {
        if (std::isdigit(*c)) {
            advance();
        } else {
            break;
        }
    }

    if (peek().value_or(char{}) == '.' && std::isdigit(peekNext().value_or(char{}))) {
        advance();
        while (const auto c = peek()) {
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

auto Scanner::string() noexcept -> Token
{
    while (true) {
        if (const auto c = peek()) {
            if (*c == '"' && peekPrevious().value_or(char{}) != '\\') {
                break;
            }

            if (*c == '\n') {
                m_line++;
                m_column = 0_uz;
            }

            advance();
        } else {
            return {TokenType::Error, m_line, m_column, "Unterminated string"};
        }
    }

    advance();
    return makeToken(TokenType::String);
}

auto Scanner::makeToken(TokenType tokenType) noexcept -> Token
{
    auto length = m_current - m_start;
    return {tokenType, m_line, m_column - length, std::string_view{m_code.data() + m_start, length}};
}
}   // namespace poise::scanner
