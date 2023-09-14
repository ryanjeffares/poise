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

        auto skipWhitespace() -> void;
        auto advance() -> std::optional<char>;

        auto peek() -> std::optional<char>;
        auto peekNext() -> std::optional<char>;
        auto peekPrevious() -> std::optional<char>;

        auto scanToken() -> Token;

        auto identifier() -> Token;
        auto number() -> Token;
        auto string() -> Token;

        auto makeToken(TokenType tokenType) -> Token;

    private:
        std::string m_code;

        std::size_t m_start{}, m_current{};
        std::size_t m_line{1}, m_column{1};

        std::unordered_map<char, TokenType> m_symbolLookup;
        std::unordered_map<std::string_view, TokenType> m_keywordLookup;
    };
}
