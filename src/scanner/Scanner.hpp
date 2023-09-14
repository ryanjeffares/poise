#ifndef POISE_SCANNER_HPP
#define POISE_SCANNER_HPP

#include "Token.hpp"

#include <cstdlib>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace poise::scanner
{
    class Scanner
    {
    public:
        Scanner(std::filesystem::path inFilePath);

        [[nodiscard]] auto getCodeAtLine(std::size_t line) const -> std::string_view;
        [[nodiscard]] auto getNumLines() const -> std::size_t;
        [[nodiscard]] auto scanToken() -> Token;

    private:
        auto skipWhitespace() -> void;
        auto advance() -> std::optional<char>;

        [[nodiscard]] auto peek() -> std::optional<char>;
        [[nodiscard]] auto peekNext() -> std::optional<char>;
        [[nodiscard]] auto peekPrevious() -> std::optional<char>;

        [[nodiscard]] auto identifier() -> Token;
        [[nodiscard]] auto number() -> Token;
        [[nodiscard]] auto string() -> Token;

        [[nodiscard]] auto makeToken(TokenType tokenType) -> Token;

        std::string m_code;

        std::size_t m_start{}, m_current{};
        std::size_t m_line{1zu}, m_column{0zu};

        std::unordered_map<char, TokenType> m_symbolLookup;
        std::unordered_map<std::string_view, TokenType> m_keywordLookup;
    };
}

#endif
