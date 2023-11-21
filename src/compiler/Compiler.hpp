#ifndef POISE_COMPILER_HPP
#define POISE_COMPILER_HPP

#include "../poise.hpp"

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
        FileError,
        ParseError,
    };

    class Compiler
    {
    public:
        Compiler(runtime::Vm* vm, std::filesystem::path inFilePath);

        [[nodiscard]] auto compile() -> CompileResult;

    private:
        auto emitOp(runtime::Op op, usize line) -> void;
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

        auto logicOr() -> void;
        auto logicAnd() -> void;
        auto bitwiseOr() -> void;
        auto bitwiseXor() -> void;
        auto bitwiseAnd() -> void;
        auto equality() -> void;
        auto comparison() -> void;
        auto shift() -> void;
        auto term() -> void;
        auto factor() -> void;
        auto unary() -> void;
        auto exponent() -> void;
        auto primary() -> void;

        auto parseString() -> void;
        auto parseInt() -> void;
        auto parseFloat() -> void;

        auto errorAtCurrent(std::string_view message) -> void;
        auto errorAtPrevious(std::string_view message) -> void;
        auto error(const scanner::Token& token, std::string_view message) -> void;

        bool m_hadError{};

        scanner::Scanner m_scanner;
        std::filesystem::path m_filePath;
        std::optional<scanner::Token> m_previous, m_current;

        runtime::Vm* m_vm;
        std::optional<runtime::Value> m_mainFunction{};
    };
}

#endif
