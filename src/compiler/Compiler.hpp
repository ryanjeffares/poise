#ifndef POISE_COMPILER_HPP
#define POISE_COMPILER_HPP

#include "../runtime/Op.hpp"
#include "../runtime/Vm.hpp"
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
        Compiler(runtime::Vm* vm, std::filesystem::path inFilePath);

        [[nodiscard]] auto compile() -> CompileResult;

    private:
        auto emitOp(runtime::Op op, std::size_t line) -> void;
        auto emitConstant(runtime::Value value) -> void;

        auto advance() -> void;
        [[nodiscard]] auto match(scanner::TokenType expected) -> bool;
        [[nodiscard]] auto check(scanner::TokenType expected) -> bool;

        auto declaration() -> void;
        auto funcDeclaration() -> void;

        auto statement() -> void;
        auto expressionStatement() -> void;
        auto printLnStatement() -> void;

        auto expression() -> void;
        auto string() -> void;

        auto errorAtCurrent(const std::string& message) -> void;
        auto errorAtPrevious(const std::string& message) -> void;
        auto error(const scanner::Token& token, const std::string& message) -> void;

        std::filesystem::path m_filePath;
        scanner::Scanner m_scanner;
        std::optional<scanner::Token> m_previous, m_current;

        runtime::Vm* m_vm;

        bool m_hadError{};
    };
}

#endif
