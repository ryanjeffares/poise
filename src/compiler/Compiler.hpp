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
        enum class Context
        {
            TopLevel, Function,
        };

        auto emitOp(runtime::Op op, usize line) -> void;
        auto emitConstant(runtime::Value value) -> void;

        struct JumpIndexes
        {
            usize constantIndex, opIndex;
        };

        auto emitJump(bool jumpCondition) -> JumpIndexes;
        auto patchJump(JumpIndexes jumpIndexes) -> void;

        auto advance() -> void;
        [[nodiscard]] auto match(scanner::TokenType expected) -> bool;
        [[nodiscard]] auto check(scanner::TokenType expected) -> bool;

        auto declaration() -> void;
        auto funcDeclaration() -> void;
        auto varDeclaration(bool isFinal) -> void;

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
        auto call() -> void;
        auto primary() -> void;
        auto identifier() -> void;

        auto parseString() -> void;
        auto parseInt() -> void;
        auto parseFloat() -> void;

        [[nodiscard]] auto parseCallArgs() -> u8;
        [[nodiscard]] auto parseFunctionArgs() -> u8;

        auto errorAtCurrent(std::string_view message) -> void;
        auto errorAtPrevious(std::string_view message) -> void;
        auto error(const scanner::Token& token, std::string_view message) -> void;

        bool m_hadError{};

        scanner::Scanner m_scanner;
        std::filesystem::path m_filePath;
        std::optional<scanner::Token> m_previous, m_current;
        std::vector<Context> m_contextStack;

        struct LocalVariable
        {
            std::string name;
            bool isFinal;
        };

        std::vector<LocalVariable> m_localNames;

        runtime::Vm* m_vm;
        std::optional<runtime::Value> m_mainFunction{};
    };
}

#endif
