#ifndef POISE_SCANNER_HPP
#define POISE_SCANNER_HPP

#include "../Poise.hpp"

#include "Token.hpp"

#include <cstdlib>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace poise::scanner {
class Scanner
{
public:
    explicit Scanner(const std::filesystem::path& inFilePath);

    [[nodiscard]] auto getCodeAtLine(usize line) const -> std::string_view;
    [[nodiscard]] auto getNumLines() const -> usize;
    [[nodiscard]] auto scanToken() -> Token;

private:
    auto skipWhitespace() -> void;
    auto advance() -> std::optional<char>;

    [[nodiscard]] auto peek() -> std::optional<char>;
    [[nodiscard]] auto peekNext() -> std::optional<char>;
    [[nodiscard]] auto peekPrevious() -> std::optional<char>;

    [[nodiscard]] auto multiCharSymbol(const std::unordered_map<char, TokenType>& pairs, TokenType defaultType) -> Token;
    [[nodiscard]] auto identifier() -> Token;
    [[nodiscard]] auto number() -> Token;
    [[nodiscard]] auto string() -> Token;

    [[nodiscard]] auto makeToken(TokenType tokenType) -> Token;

private:
    std::string m_code;

    usize m_start{}, m_current{};
    usize m_line{1_uz}, m_column{0_uz};

    std::unordered_map<char, TokenType> m_symbolLookup;
    std::unordered_map<std::string_view, TokenType> m_keywordLookup;
};  // class Scanner
}   // namespace poise::scanner

#endif  // #ifndef POISE_SCANNER_HPP
