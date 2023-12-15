#ifndef POISE_COMPILER_HPP
#define POISE_COMPILER_HPP

#include "../Poise.hpp"

#include "../objects/PoiseType.hpp"
#include "../runtime/Op.hpp"
#include "../runtime/Vm.hpp"
#include "../scanner/Scanner.hpp"

#include <filesystem>
#include <optional>
#include <memory>
#include <string>

namespace poise::compiler {
class Compiler
{
public:
    enum class CompileResult
    {
        Success, CompileError, FileError, ParseError,
    };

    Compiler(bool mainFile, bool stdFile, runtime::Vm* vm, std::filesystem::path inFilePath);

    [[nodiscard]] auto compile() -> CompileResult;
    [[nodiscard]] auto scanner() const noexcept -> const scanner::Scanner*;

private:
    enum class Context
    {
        ForLoop, Function, IfStatement, TopLevel, TryCatch, WhileLoop,
    };

    auto emitOp(runtime::Op op, usize line) const noexcept -> void;
    auto emitConstant(runtime::Value value) const noexcept -> void;

    struct JumpIndexes
    {
        usize constantIndex, opIndex;
    };

    enum class JumpType
    {
        IfFalse, IfTrue, None
    };

    auto emitJump() const noexcept -> JumpIndexes;
    auto emitJump(JumpType jumpType, bool emitPop) const noexcept -> JumpIndexes;
    auto patchJump(JumpIndexes jumpIndexes) const noexcept -> void;

    auto advance() -> void;
    [[nodiscard]] auto match(scanner::TokenType expected) -> bool;
    [[nodiscard]] auto check(scanner::TokenType expected) const noexcept -> bool;

    struct LocalVariable
    {
        std::string name;
        bool isFinal;
    };

    [[nodiscard]] auto hasLocal(std::string_view localName) const noexcept -> bool;
    [[nodiscard]] auto findLocal(std::string_view localName) const noexcept -> std::optional<LocalVariable>;
    [[nodiscard]] auto indexOfLocal(std::string_view localName) const noexcept -> std::optional<usize>;

    struct NamespaceParseResult
    {
        std::filesystem::path path;
        std::string name;
        bool isStdFile;
    };

    struct FunctionArgsParseResult
    {
        u8 numArgs;
        std::optional<runtime::types::Type> extensionFunctionType;
    };

    [[nodiscard]] auto parseCallArgs(scanner::TokenType sentinel) -> std::optional<u8>;
    [[nodiscard]] auto parseFunctionArgs(bool isLambda) -> std::optional<FunctionArgsParseResult>;
    [[nodiscard]] auto parseNamespaceImport() -> std::optional<NamespaceParseResult>;
    [[nodiscard]] auto parseBlock(std::string_view scopeType) -> bool;

    auto declaration() -> void;
    auto importDeclaration() -> void;
    auto funcDeclaration(bool isExported) -> void;
    auto varDeclaration(bool isFinal) -> void;

    auto statement() -> void;
    auto expressionStatement() -> void;
    auto printStatement(bool err, bool newLine) -> void;
    auto returnStatement() -> void;
    auto tryStatement() -> void;
    auto catchStatement() -> void;
    auto ifStatement() -> void;
    auto whileStatement() -> void;
    auto forStatement() -> void;

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
    auto nativeCall() -> void;
    auto namespaceQualifiedCall() -> void;

    auto typeIdent() -> void;
    auto typeOf() -> void;
    auto lambda() -> void;
    auto list() -> void;

    auto parseString() -> void;
    auto parseInt() -> void;
    auto parseFloat() -> void;

    auto errorAtCurrent(std::string_view message) -> void;
    auto errorAtPrevious(std::string_view message) -> void;
    auto error(const scanner::Token& token, std::string_view message) -> void;

private:
    bool m_mainFile{};
    bool m_stdFile{};
    bool m_hadError{};
    bool m_passedImports{};

    std::unordered_map<std::string, std::filesystem::path> m_importAliasLookup;
    std::unique_ptr<Compiler> m_importCompiler;

    scanner::Scanner m_scanner;
    runtime::Vm* m_vm;
    std::filesystem::path m_filePath;
    runtime::NamespaceManager::NamespaceHash m_filePathHash;
    std::optional<scanner::Token> m_previous, m_current;
    std::vector<Context> m_contextStack;

    std::vector<LocalVariable> m_localNames;

    std::optional<runtime::Value> m_mainFunction{};

    std::hash<std::string> m_stringHasher;
};  // class Compiler
}   // namespace poise::compiler

#endif  // #ifndef POISE_COMPILER_HPP
