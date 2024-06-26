#ifndef POISE_COMPILER_HPP
#define POISE_COMPILER_HPP

#include "../Poise.hpp"

#include "../runtime/Op.hpp"
#include "../runtime/Vm.hpp"
#include "../scanner/Scanner.hpp"

#include <filesystem>
#include <optional>
#include <stack>
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

private:
    enum class Context
    {
        Catch, ForLoop, Function, IfStatement, Lambda, TopLevel, Try, WhileLoop,
    };

    auto emitOp(runtime::Op op, usize line) const noexcept -> void;
    auto emitConstant(runtime::Value value) const noexcept -> void;

    struct JumpIndexes
    {
        usize constantIndex, opIndex;
    };

    enum class JumpType
    {
        IfFalse, IfTrue, Jump
    };

    [[nodiscard]] auto emitJump() const noexcept -> JumpIndexes;
    [[nodiscard]] auto emitJump(JumpType jumpType, bool emitPop) const noexcept -> JumpIndexes;
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

    [[nodiscard]] auto checkLastOp(runtime::Op op) const noexcept -> bool;
    [[nodiscard]] auto lastOpWasAssignment() const noexcept -> bool;

    struct CallArgsParseResult
    {
        u8 numArgs;
        bool hasUnpack;
    };

    struct FunctionParamsParseResult
    {
        u8 numParams;
        bool hasVariadicParams;
        std::vector<runtime::types::Type> extensionFunctionTypes;
    };

    struct NamespaceImportParseResult
    {
        std::filesystem::path path;
        std::string name;
        bool isStdFile;
    };

    struct NamespaceQualificationParseResult
    {
        std::string namespaceText;
        usize namespaceHash;
    };

    [[nodiscard]] auto checkNameCollisions(std::string_view structConstFuncName) -> bool;
    [[nodiscard]] auto parseCallArgs(scanner::TokenType sentinel) -> std::optional<CallArgsParseResult>;
    [[nodiscard]] auto parseFunctionParams(bool isLambda) -> std::optional<FunctionParamsParseResult>;
    [[nodiscard]] auto parseNamespaceImport() -> std::optional<std::vector<NamespaceImportParseResult>>;
    [[nodiscard]] auto parseNamespaceQualification() -> std::optional<NamespaceQualificationParseResult>;
    [[nodiscard]] auto parseBlock(std::string_view scopeType) -> bool;
    auto parseTypeAnnotation() -> void;

    auto declaration() -> void;
    auto importDeclaration() -> void;
    auto funcDeclaration(bool isExported) -> void;
    auto varDeclaration(bool isFinal) -> void;
    auto constDeclaration(bool isExported) -> void;
    auto structDeclaration(bool isExported) -> void;

    auto statement(bool consumeSemicolon) -> void;
    auto expressionStatement(bool consumeSemicolon) -> void;
    auto assertStatement(bool consumeSemicolon) -> void;
    auto printStatement(bool err, bool newLine, bool consumeSemicolon) -> void;
    auto returnStatement(bool consumeSemicolon) -> void;
    auto throwStatement(bool consumeSemicolon) -> void;
    auto tryStatement() -> void;
    auto catchStatement() -> void;
    auto ifStatement() -> void;
    auto whileStatement() -> void;
    auto forStatement() -> void;
    auto breakStatement() -> void;
    auto continueStatement() -> void;

    auto expression(bool canAssign, bool canUnpack) -> void;
    auto unpack() -> void;
    auto range(bool canAssign) -> void;
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

    [[nodiscard]] auto constantExpression() -> std::optional<runtime::Value>;
    [[nodiscard]] auto constantLogicOr() -> std::optional<runtime::Value>;
    [[nodiscard]] auto constantLogicAnd() -> std::optional<runtime::Value>;
    [[nodiscard]] auto constantBitwiseOr() -> std::optional<runtime::Value>;
    [[nodiscard]] auto constantBitwiseXor() -> std::optional<runtime::Value>;
    [[nodiscard]] auto constantBitwiseAnd() -> std::optional<runtime::Value>;
    [[nodiscard]] auto constantEquality() -> std::optional<runtime::Value>;
    [[nodiscard]] auto constantComparison() -> std::optional<runtime::Value>;
    [[nodiscard]] auto constantShift() -> std::optional<runtime::Value>;
    [[nodiscard]] auto constantTerm() -> std::optional<runtime::Value>;
    [[nodiscard]] auto constantFactor() -> std::optional<runtime::Value>;
    [[nodiscard]] auto constantUnary() -> std::optional<runtime::Value>;
    [[nodiscard]] auto constantPrimary() -> std::optional<runtime::Value>;
    [[nodiscard]] auto constantNamespaceQualifiedCall() -> std::optional<runtime::Value>;

    auto identifier(bool canAssign) -> void;
    auto nativeCall() -> void;
    auto namespaceQualifiedCall() -> void;

    auto typeIdent() -> void;
    auto typeOf() -> void;
    auto lambda() -> void;
    auto list() -> void;
    auto tupleOrGrouping() -> void;
    auto dict() -> void;

    [[nodiscard]] auto parseString() -> std::optional<std::string>;
    [[nodiscard]] auto parseInt() -> std::optional<i64>;
    [[nodiscard]] auto parseFloat() -> std::optional<f64>;

    auto errorAtCurrent(std::string_view message) -> void;
    auto errorAtPrevious(std::string_view message) -> void;
    auto error(const scanner::Token& token, std::string_view message) -> void;

private:
    std::hash<std::string> m_stringHasher{};
    std::hash<std::filesystem::path> m_pathHasher{};

    bool m_mainFile{};
    bool m_stdFile{};
    bool m_hadError{};
    bool m_passedImports{};

    std::unordered_map<std::string, std::filesystem::path> m_importAliasLookup;

    scanner::Scanner m_scanner;
    runtime::Vm* m_vm;
    std::filesystem::path m_filePath;
    usize m_filePathHash;
    std::optional<scanner::Token> m_previous, m_current;
    std::vector<Context> m_contextStack;

    std::stack<std::vector<JumpIndexes>> m_breakJumpIndexesStack, m_continueJumpIndexesStack;

    std::vector<LocalVariable> m_localNames;

    std::optional<runtime::Value> m_mainFunction{};
};  // class Compiler
}   // namespace poise::compiler

#endif  // #ifndef POISE_COMPILER_HPP
