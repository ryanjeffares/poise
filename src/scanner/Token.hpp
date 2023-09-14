#include <fmt/core.h>

#include <cstdlib>
#include <string_view>

namespace poise::scanner
{
    enum class TokenType
    {
        // keywords
        End,
        Func,
        PrintLn,

        // symbols
        OpenParen,
        CloseParen,
        Colon,
        Semicolon,

        // value types
        Float,
        Identifier,
        Int,
        String,

        // other
        EndOfFile,
        Error,
    };

    class Token
    {
    public:
        Token(TokenType tokenType, std::string_view text)
            : m_tokenType{tokenType}
            , m_text{text}
        {

        }

        auto length() const -> std::size_t;
        auto tokenType() const -> TokenType;
        auto text() const -> std::string_view;
        auto print() const -> void;

    private:
        TokenType m_tokenType;
        std::string_view m_text;
    };
}

namespace fmt
{
    template<>
    struct formatter<poise::scanner::TokenType> : formatter<string_view>
    {
        auto format(poise::scanner::TokenType tokenType, format_context& context) const;
    };
}
