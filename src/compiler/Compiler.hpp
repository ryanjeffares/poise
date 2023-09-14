#include "../scanner/Scanner.hpp"

#include <filesystem>
#include <optional>
#include <string>

namespace poise::compiler
{
    enum CompileResult
    {
        Success,
        CompileError,
        ParseError,
    };

    class Compiler
    {
    public:
        Compiler(std::filesystem::path inFilePath);

        auto compile() -> CompileResult;

    private:
        auto advance() -> void;
        auto match(scanner::TokenType expected) -> bool;
        auto check(scanner::TokenType expected) -> bool;

        auto declaration() -> void;
        auto funcDeclaration() -> void;

        auto statement() -> void;
        auto expressionStatement() -> void;
        auto printLnStatement() -> void;

        auto expression() -> void;
        auto string() -> void;

        auto errorAtCurrent(const std::string& message) -> void;
        auto errorAtPrevious(const std::string& message) -> void;
        auto error() -> void;

        scanner::Scanner m_scanner;
        std::optional<scanner::Token> m_previous, m_current;
    };
}
