#include "Scanner.hpp"

#include <cctype>
#include <fstream>
#include <sstream>

namespace poise::scanner {
static std::unordered_map<std::filesystem::path, std::string> s_fileContentLookup;

Scanner::Scanner(const std::filesystem::path& inFilePath)
    : m_symbolLookup{
        {'&', TokenType::Ampersand},
        {'^', TokenType::Caret},
        {'}', TokenType::CloseBrace},
        {')', TokenType::CloseParen},
        {']', TokenType::CloseSquareBracket},
        {':', TokenType::Colon},
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
        {"break", TokenType::Break},
        {"by", TokenType::By},
        {"catch", TokenType::Catch},
        {"continue", TokenType::Continue},
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
        {"throw", TokenType::Throw},
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
        {"Dict", TokenType::DictIdent},
        {"Exception", TokenType::ExceptionIdent},
        {"Function", TokenType::FunctionIdent},
        {"List", TokenType::ListIdent},
        {"Range", TokenType::RangeIdent},
        {"Set", TokenType::SetIdent},
        {"Tuple", TokenType::TupleIdent},
    }
{
    std::ifstream inFileStream{inFilePath};
    std::stringstream inCodeStream;
    inCodeStream << inFileStream.rdbuf();
    auto codeString = inCodeStream.str();

    if (!codeString.empty() && codeString.back() != '\n') {
        // hack to make our compiler errors not throw assertion failures in fmt
        // if the error is on the last line of the file
        // and there's no trailing newline
        codeString.push_back('\n');
    }

    s_fileContentLookup[inFilePath] = std::move(codeString);
    m_code = s_fileContentLookup[inFilePath];

}

auto Scanner::getCodeAtLine(const std::filesystem::path& filePath, usize line) -> std::string
{
    // why doesn't this work if I return a string_view?
    auto current = 1_uz;
    auto strIndex = 0_uz;
    const auto codeString = s_fileContentLookup[filePath];

    while (current < line) {
        if (strIndex > codeString.length()) {
            return "";
        }

        strIndex++;

        if (codeString[strIndex - 1_uz] == '\n') {
            current++;
        }
    }

    const auto pos = codeString.find('\n', strIndex);
    return {codeString.data() + strIndex, pos - strIndex};
}

auto Scanner::getNumLines(const std::filesystem::path& filePath) noexcept -> usize
{
    auto count = 0_uz;

    for (const auto codeString = s_fileContentLookup[filePath]; const auto i : codeString) {
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
                return multiCharSymbol({{'=', TokenType::NotEqual}}, TokenType::Exclamation);
            case '=':
                return multiCharSymbol({{'=', TokenType::EqualEqual}, {'>', TokenType::Arrow}}, TokenType::Equal);
            case '<':
                return multiCharSymbol({{'<', TokenType::ShiftLeft}, {'=', TokenType::LessEqual}}, TokenType::Less);
            case '>':
                return multiCharSymbol({{'>', TokenType::ShiftRight}, {'=', TokenType::GreaterEqual}}, TokenType::Greater);
            case '.': {
                return multiCharSymbol({
                    {std::make_pair('.', '.'), TokenType::DotDotDot},
                    {std::make_pair('.', '='), TokenType::DotDotEqual},
                    {'.', TokenType::DotDot},
                }, TokenType::Dot);
            }
            case '"':
                return string();
            case ':':
                return multiCharSymbol({{':', TokenType::ColonColon}}, TokenType::Colon);
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
                if (peekNext() == '/') {
                    while (peek() != '\n') {
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
    }

    m_current++;
    m_column++;
    return m_code[m_current - 1_uz];
}

auto Scanner::peek() const noexcept -> std::optional<char>
{
    if (m_current >= m_code.length()) {
        return {};
    }

    return m_code[m_current];
}

auto Scanner::peekNext() const noexcept -> std::optional<char>
{
    if (m_current >= m_code.length() - 1_uz) {
        return {};
    }

    return m_code[m_current + 1_uz];
}

auto Scanner::peekPrevious() const noexcept -> std::optional<char>
{
    if (m_current == 0_uz) {
        return {};
    }

    return m_code[m_current - 1_uz];
}

auto Scanner::multiCharSymbol(std::initializer_list<const MultiCharMatch> matches, TokenType defaultType) noexcept -> Token
{
    for (const auto& [match, tokenType] : matches) {
        if (match.index() == 0) {
            if (const auto c = std::get<0>(match); peek() == c) {
                advance();
                return makeToken(tokenType);
            }
        } else if (match.index() == 1) {
            if (auto [c1, c2] = std::get<1>(match); peek() == c1 && peekNext() == c2) {
                advance();
                advance();
                return makeToken(tokenType);
            }
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

    if (peek() == '.' && peekNext() && std::isdigit(*peekNext())) {
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
            if (*c == '"' && peekPrevious() != '\\') {
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
    const auto length = m_current - m_start;
    return {tokenType, m_line, m_column - length, std::string_view{m_code.data() + m_start, length}};
}
}   // namespace poise::scanner
