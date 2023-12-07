#ifndef POISE_COMPILER_HPP
#define POISE_COMPILER_HPP

#include "../Poise.hpp"

#include "../runtime/Op.hpp"
#include "../runtime/Vm.hpp"
#include "../scanner/Scanner.hpp"

#include <filesystem>
#include <optional>
#include <string>

namespace poise::compiler {
enum class CompileResult
{
    Success, CompileError, FileError, ParseError,
};

class Compiler
{
public:
    Compiler(runtime::Vm* vm, std::filesystem::path inFilePath);

    [[nodiscard]] auto compile() -> CompileResult;
    [[nodiscard]] auto scanner() const -> const scanner::Scanner*;

private:
    enum class Context
    {
        Function, TopLevel, TryCatch,
    };

    auto emitOp(runtime::Op op, usize line) -> void;
    auto emitConstant(runtime::Value value) -> void;

    struct JumpIndexes
    {
        usize constantIndex, opIndex;
        bool emitPop;
    };

    enum class JumpType
    {
        IfFalse, IfTrue, None
    };

    auto emitJump() -> JumpIndexes;
    auto emitJump(JumpType jumpType, bool emitPop) -> JumpIndexes;
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
    auto returnStatement() -> void;
    auto tryStatement() -> void;
    auto catchStatement() -> void;
    auto ifStatement() -> void;
    auto whileStatement() -> void;

    auto expression(bool canAssign) -> void;
    auto logicOr(bool canAssign) -> void;
    auto logicAnd(bool canAssign) -> void;
    auto bitwiseOr(bool canAssign) -> void;
    auto bitwiseXor(bool canAssign) -> void;
    auto bitwiseAnd(bool canAssign) -> void;
    auto equality(bool canAssign) -> void;
    auto comparison(bool canAssign) -> void;
    auto shift(bool canAssign) -> void;
    auto term(bool canAssign) -> void;
    auto factor(bool canAssign) -> void;
    auto unary(bool canAssign) -> void;
    auto call(bool canAssign) -> void;
    auto primary(bool canAssign) -> void;

    auto identifier(bool canAssign) -> void;
    auto typeIdent() -> void;
    auto typeOf() -> void;
    auto lambda() -> void;
    auto parseString() -> void;
    auto parseInt() -> void;
    auto parseFloat() -> void;

    [[nodiscard]] auto parseCallArgs() -> u8;
    [[nodiscard]] auto parseFunctionArgs() -> u8;
    [[nodiscard]] auto block(std::string_view scopeType) -> bool;

    auto errorAtCurrent(std::string_view message) -> void;
    auto errorAtPrevious(std::string_view message) -> void;
    auto error(const scanner::Token& token, std::string_view message) -> void;

private:
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
};  // class Compiler
}   // namespace poise::compiler

#endif  // #ifndef POISE_COMPILER_HPP
