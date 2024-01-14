#ifndef POISE_SCANNER_HPP
#define POISE_SCANNER_HPP

#include "../Poise.hpp"

#include "Token.hpp"

#include <filesystem>
#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

namespace poise::scanner {
class Scanner
{
public:
    explicit Scanner(const std::filesystem::path& inFilePath);

    [[nodiscard]] static auto getCodeAtLine(const std::filesystem::path& filePath, usize line) -> std::string;
    [[nodiscard]] static auto getNumLines(const std::filesystem::path& filePath) noexcept -> usize;
    [[nodiscard]] auto scanToken() noexcept -> Token;

private:
    auto skipWhitespace() noexcept -> void;
    auto advance() noexcept -> std::optional<char>;

    [[nodiscard]] auto peek() const noexcept -> std::optional<char>;
    [[nodiscard]] auto peekNext() const noexcept -> std::optional<char>;
    [[nodiscard]] auto peekPrevious() const noexcept -> std::optional<char>;

    struct MultiCharMatch
    {
        std::variant<char, std::pair<char, char>> matches;
        TokenType tokenType;
    };

    [[nodiscard]] auto multiCharSymbol(std::initializer_list<const MultiCharMatch> matches, TokenType defaultType) noexcept -> Token;

    [[nodiscard]] auto identifier() noexcept -> Token;
    [[nodiscard]] auto number() noexcept -> Token;
    [[nodiscard]] auto string() noexcept -> Token;

    [[nodiscard]] auto makeToken(TokenType tokenType) noexcept -> Token;

    std::string_view m_code;

    usize m_start{}, m_current{};
    usize m_line{1_uz}, m_column{0_uz};

    std::unordered_map<char, TokenType> m_symbolLookup;
    std::unordered_map<std::string_view, TokenType> m_keywordLookup;
};  // class Scanner
}   // namespace poise::scanner

#endif  // #ifndef POISE_SCANNER_HPP
