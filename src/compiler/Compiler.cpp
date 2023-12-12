#include "Compiler.hpp"

#include <fmt/color.h>
#include <fmt/core.h>

#include <charconv>
#include <limits>

#define RETURN_IF_NO_MATCH(tokenType, message)          \
    do {                                                \
        if (!match(tokenType)) {                        \
            errorAtCurrent(message);                    \
            return;                                     \
        }                                               \
    } while (false)

#define RETURN_VALUE_IF_NO_MATCH(tokenType, message, returnValue)   \
    do {                                                            \
        if (!match(tokenType)) {                                    \
            errorAtCurrent(message);                                \
            return returnValue;                                     \
        }                                                           \
    } while (false)

#define EXPECT_SEMICOLON() RETURN_IF_NO_MATCH(scanner::TokenType::Semicolon, "Expected ';'")
#define EXPECT_SEMICOLON_RETURN_VALUE(returnValue) RETURN_VALUE_IF_NO_MATCH(scanner::TokenType::Semicolon, "Expected ';'", returnValue)

namespace poise::compiler {
Compiler::Compiler(bool mainFile, bool stdFile, runtime::Vm* vm, std::filesystem::path inFilePath)
    : m_mainFile{mainFile}
    , m_stdFile{stdFile}
    , m_scanner{inFilePath}
    , m_vm{vm}
    , m_filePath{std::move(inFilePath)}
    , m_filePathHash{m_vm->namespaceManager()->namespaceHash(m_filePath)}
{

}

auto Compiler::compile() -> CompileResult
{
    if (!std::filesystem::exists(m_filePath) || m_filePath.extension() != ".poise") {
        return CompileResult::FileError;
    }

    if (m_mainFile) {
        [[maybe_unused]] const auto _ = m_vm->namespaceManager()->addNamespace(m_filePath, "entry", std::nullopt);
    }

    m_contextStack.push_back(Context::TopLevel);

    advance();

    while (true) {
        if (m_hadError) {
            break;
        }

        if (check(scanner::TokenType::Error)) {
            return CompileResult::ParseError;
        }

        if (check(scanner::TokenType::EndOfFile)) {
            break;
        }

        declaration();
    }

    if (m_hadError) {
        return CompileResult::CompileError;
    }

    if (m_mainFile) {
        if (m_mainFunction) {
            emitConstant(m_filePathHash);
            emitConstant(m_stringHasher("main"));
            emitConstant("main");
            emitOp(runtime::Op::LoadFunction, 0_uz);
            emitConstant(0);
            emitOp(runtime::Op::Call, 0_uz);
            emitOp(runtime::Op::Pop, 0_uz);
            emitOp(runtime::Op::Exit, m_scanner.getNumLines());
        } else {
            errorAtPrevious("No main function declared");
            return CompileResult::CompileError;
        }
    }

    return CompileResult::Success;
}

auto Compiler::scanner() const noexcept -> const scanner::Scanner*
{
    return &m_scanner;
}

auto Compiler::emitOp(runtime::Op op, usize line) noexcept -> void
{
    m_vm->emitOp(op, line);
}

auto Compiler::emitConstant(runtime::Value value) noexcept -> void
{
    m_vm->emitConstant(std::move(value));
}

auto Compiler::emitJump() noexcept -> JumpIndexes
{
    return emitJump(JumpType::None, false);
}

auto Compiler::emitJump(JumpType jumpType, bool emitPop) noexcept -> JumpIndexes
{
    const auto function = m_vm->currentFunction();

    switch (jumpType) {
        case JumpType::IfFalse:
            emitOp(runtime::Op::JumpIfFalse, m_previous->line());
            break;
        case JumpType::IfTrue:
            emitOp(runtime::Op::JumpIfTrue, m_previous->line());
            break;
        case JumpType::None:
            emitOp(runtime::Op::Jump, m_previous->line());
            break;
    }

    const auto jumpConstantIndex = function->numConstants();
    emitConstant(0_uz);
    const auto jumpOpIndex = function->numConstants();
    emitConstant(0_uz);

    if (jumpType != JumpType::None) {
        emitConstant(emitPop);
    }

    return {jumpConstantIndex, jumpOpIndex};
}

auto Compiler::patchJump(JumpIndexes jumpIndexes) noexcept -> void
{
    const auto function = m_vm->currentFunction();

    const auto numOps = function->numOps();
    const auto numConstants = function->numConstants();
    function->setConstant(numOps, jumpIndexes.opIndex);
    function->setConstant(numConstants, jumpIndexes.constantIndex);
}

auto Compiler::advance() -> void
{
    m_previous = m_current;
    m_current = m_scanner.scanToken();

#ifdef POISE_DEBUG
    m_current->print();
#endif

    if (m_current->tokenType() == scanner::TokenType::Error) {
        errorAtCurrent("Invalid token");
    }
}

auto Compiler::match(scanner::TokenType expected) -> bool
{
    if (!check(expected)) {
        return false;
    }

    advance();
    return true;
}

auto Compiler::check(scanner::TokenType expected) -> bool
{
    return m_current && m_current->tokenType() == expected;
}

auto Compiler::declaration() -> void
{
    if (match(scanner::TokenType::Import)) {
        importDeclaration();
    } else if (match(scanner::TokenType::Func)) {
        funcDeclaration(false);
    } else if (match(scanner::TokenType::Var)) {
        varDeclaration(false);
    } else if (match(scanner::TokenType::Final)) {
        varDeclaration(true);
    } else if (match(scanner::TokenType::Export)) {
        if (match(scanner::TokenType::Func)) {
            funcDeclaration(true);
        } else {
            errorAtCurrent("Expected function");
        }
    } else {
        statement();
    }
}

auto Compiler::importDeclaration() -> void
{
    if (m_contextStack.back() != Context::TopLevel) {
        errorAtPrevious("Import only allowed at top level");
        return;
    }

    if (m_passedImports) {
        errorAtPrevious("Imports must only appear before any other top level declarations");
        return;
    }

    RETURN_IF_NO_MATCH(scanner::TokenType::Identifier, "Expected namespace");

    const auto namespaceParseRes = parseNamespaceImport();
    if (!namespaceParseRes) {
        return;
    }

    const auto& [path, name, isStdFile] = *namespaceParseRes;

    if (!std::filesystem::exists(path)) {
        errorAtPrevious(fmt::format("Cannot open file {}", path.string()));
        return;
    }

    if (m_vm->namespaceManager()->addNamespace(path, name, m_filePathHash)) {
        m_importCompiler = std::make_unique<Compiler>(false, isStdFile, m_vm, path);
        const auto res = m_importCompiler->compile();
        m_importCompiler.reset();
        if (res != CompileResult::Success) {
            // set the error flag here, so we stop compiling
            // but no need to report - the import compiler already did this
            m_hadError = true;
        }
    }
}

auto Compiler::funcDeclaration(bool isExported) -> void
{
    if (m_contextStack.back() != Context::TopLevel) {
        errorAtPrevious("Function declaration only allowed at top level");
        return;
    }

    m_contextStack.push_back(Context::Function);

    RETURN_IF_NO_MATCH(scanner::TokenType::Identifier, "Expected function name");
    auto functionName = m_previous->string();

    if (functionName.starts_with("__")) {
        errorAtPrevious("Function names may not start with '__' as this is reserved for the standard library");
        return;
    }

    RETURN_IF_NO_MATCH(scanner::TokenType::OpenParen, "Expected '(' after function name");
    const auto args = parseFunctionArgs(false);
    if (!args) {
        return;
    }

    const auto [numArgs, extensionFunctionType] = *args;

    RETURN_IF_NO_MATCH(scanner::TokenType::OpenBrace, "Expected '{' after function signature");

    auto prevFunction = m_vm->currentFunction();

    auto function = runtime::Value::createObject<objects::PoiseFunction>(std::move(functionName), m_filePath.string(), m_filePathHash, numArgs, isExported);
    auto functionPtr = function.object()->asFunction();
    m_vm->setCurrentFunction(functionPtr);

    if (!parseBlock("function")) {
        return;
    }

    if (functionPtr->opList().empty() || functionPtr->opList().back().op != runtime::Op::Return) {
        // if no return statement, make sure we pop locals and implicitly return none
        emitConstant(m_localNames.size());
        emitOp(runtime::Op::PopLocals, m_previous->line());
        emitConstant(runtime::Value::none());
        emitOp(runtime::Op::LoadConstant, m_previous->line());
        emitOp(runtime::Op::Return, m_previous->line());
    }

    m_vm->setCurrentFunction(prevFunction);

    if (functionPtr->name() == "main") {
        m_mainFunction = function;
    }

#ifdef POISE_DEBUG
    functionPtr->printOps();
#endif

    if (extensionFunctionType) {
        types::associateExtensionFunction(*extensionFunctionType, function);
    }

    m_vm->namespaceManager()->addFunctionToNamespace(m_filePathHash, std::move(function));

    m_localNames.clear();
    m_contextStack.pop_back();

    m_passedImports = true;
}

auto Compiler::varDeclaration(bool isFinal) -> void
{
    if (m_contextStack.back() == Context::TopLevel) {
        errorAtPrevious("Variable declaration not allowed at top level");
        return;
    }

    RETURN_IF_NO_MATCH(scanner::TokenType::Identifier, "Expected identifier");

    auto varName = m_previous->string();
    if (varName.starts_with("__")) {
        errorAtPrevious("Variable names may not start with '__' as this is reserved for the standard library");
        return;
    }

    if (std::find_if(m_localNames.cbegin(), m_localNames.cend(), [&varName](const LocalVariable& local) {
        return local.name == varName;
    }) != m_localNames.cend()) {
        errorAtPrevious("Local variable with the same name already declared");
        return;
    }

    m_localNames.push_back({std::move(varName), isFinal});

    if (match(scanner::TokenType::Equal)) {
        expression(false);
    } else {
        if (isFinal) {
            errorAtCurrent("Expected assignment after 'final'");
            return;
        }

        emitConstant(runtime::Value::none());
        emitOp(runtime::Op::LoadConstant, m_previous->line());
    }

    emitOp(runtime::Op::DeclareLocal, m_previous->line());

    EXPECT_SEMICOLON();
}

auto Compiler::statement() -> void
{
    if (m_contextStack.back() == Context::TopLevel) {
        errorAtCurrent("Statements not allowed at top level");
        return;
    }

    if (match(scanner::TokenType::Print) || match(scanner::TokenType::PrintLn)
        || match(scanner::TokenType::EPrint) || match(scanner::TokenType::EPrintLn)) {
        const auto err = m_previous->tokenType() == scanner::TokenType::EPrint || m_previous->tokenType() == scanner::TokenType::EPrintLn;
        const auto newLine = m_previous->tokenType() == scanner::TokenType::PrintLn || m_previous->tokenType() == scanner::TokenType::EPrintLn;
        printStatement(err, newLine);
    } else if (match(scanner::TokenType::Return)) {
        returnStatement();
    } else if (match(scanner::TokenType::Try)) {
        tryStatement();
    } else if (match(scanner::TokenType::If)) {
        ifStatement();
    } else if (match(scanner::TokenType::While)) {
        whileStatement();
    } else {
        expressionStatement();
    }
}

auto Compiler::expressionStatement() -> void
{
    /*
        calling `expression()` in any other context means the result of that expression is being consumed
        but this function is called when you have a line that's just like

        ```
        5 + 5;
        some_void_function();
        ```

        so emit an extra `Pop` instruction to remove that unused result

        OR an assignment

        ```
        var my_var = "hello";
        my_var = "goodbye"; // here
        ```

        in that case, nothing to pop if the last emitted op was assign local
    */

    expression(true);

    if (m_vm->currentFunction()->opList().back().op != runtime::Op::AssignLocal) {
        emitOp(runtime::Op::Pop, m_previous->line());
    }

    EXPECT_SEMICOLON();
}

auto Compiler::printStatement(bool err, bool newLine) -> void
{
    RETURN_IF_NO_MATCH(scanner::TokenType::OpenParen, "Expected '(' after 'println'");

    expression(false);
    emitConstant(err);
    emitConstant(newLine);
    emitOp(runtime::Op::Print, m_previous->line());

    RETURN_IF_NO_MATCH(scanner::TokenType::CloseParen, "Expected ')' after 'println'");
    EXPECT_SEMICOLON();
}

auto Compiler::returnStatement() -> void
{
    if (match(scanner::TokenType::Semicolon)) {
        // emit none value to return if no value is returned
        emitConstant(runtime::Value::none());
        emitOp(runtime::Op::LoadConstant, m_previous->line());
    } else {
        // else the return value should be any expression
        expression(false);
    }

    // pop local variables
    emitConstant(m_localNames.size());
    emitOp(runtime::Op::PopLocals, m_previous->line());

    // expression above is still on the stack
    emitOp(runtime::Op::Return, m_previous->line());

    EXPECT_SEMICOLON();
}

auto Compiler::tryStatement() -> void
{
    if (m_contextStack.back() == Context::TopLevel) {
        errorAtPrevious("'try' not allowed at top level");
        return;
    }

    const auto numLocalsStart = m_localNames.size();

    const auto function = m_vm->currentFunction();
    const auto jumpConstantIndex = function->numConstants();
    emitConstant(0_uz);
    const auto jumpOpIndex = function->numConstants();
    emitConstant(0_uz);

    emitOp(runtime::Op::EnterTry, m_previous->line());

    RETURN_IF_NO_MATCH(scanner::TokenType::OpenBrace, "Expected '{'");

    if (!parseBlock("try block")) {
        return;
    }

    // no exception thrown - PopLocals, ExitTry, Jump (to after the catch)
    // exception thrown - PopLocals

    // these instructions are in the case of no exception thrown - need to pop locals, exit the try, and jump to after the catch block
    emitConstant(m_localNames.size() - numLocalsStart);
    emitOp(runtime::Op::PopLocals, m_previous->line());
    emitOp(runtime::Op::ExitTry, m_previous->line());
    const auto jumpIndexes = emitJump();

    // this patching is in the case of an exception being thrown - need to pop locals, and then continue into the catch block
    const auto numConstants = function->numConstants();
    const auto numOps = function->numOps();
    function->setConstant(numConstants, jumpConstantIndex);
    function->setConstant(numOps, jumpOpIndex);

    emitConstant(m_localNames.size() - numLocalsStart);
    emitOp(runtime::Op::PopLocals, m_previous->line());

    m_localNames.resize(numLocalsStart);

    RETURN_IF_NO_MATCH(scanner::TokenType::Catch, "Expected 'catch' after 'try' block");
    catchStatement();

    patchJump(jumpIndexes);
}

auto Compiler::catchStatement() -> void
{
    const auto numLocalsStart = m_localNames.size();

    if (match(scanner::TokenType::Identifier)) {
        // create this as a local
        // assign the caught exception to that local
        const auto text = m_previous->text();

        if (std::find_if(m_localNames.cbegin(), m_localNames.cend(), [&text](const LocalVariable& local) {
            return local.name == text;
        }) != m_localNames.cend()) {
            errorAtPrevious("Local variable with the same name already declared");
            return;
        }

        m_localNames.push_back({m_previous->string(), false});
        emitOp(runtime::Op::DeclareLocal, m_previous->line());
    } else {
        emitOp(runtime::Op::Pop, m_previous->line());
    }

    RETURN_IF_NO_MATCH(scanner::TokenType::OpenBrace, "Expected '{'");

    if (!parseBlock("catch block")) {
        return;
    }

    emitConstant(m_localNames.size() - numLocalsStart);
    emitOp(runtime::Op::PopLocals, m_previous->line());
    m_localNames.resize(numLocalsStart);
}

auto Compiler::ifStatement() -> void
{
    if (m_contextStack.back() == Context::TopLevel) {
        errorAtPrevious("'if' not allowed at top level");
        return;
    }

    expression(false);
    RETURN_IF_NO_MATCH(scanner::TokenType::OpenBrace, "Expected '{'");

    const auto numLocalsStart = m_localNames.size();

    // jump if the condition fails, otherwise continue on and pop the condition result
    const auto falseJumpIndexes = emitJump(JumpType::IfFalse, true);

    if (!parseBlock("if statement")) {
        return;
    }

    emitConstant(m_localNames.size() - numLocalsStart);
    emitOp(runtime::Op::PopLocals, m_previous->line());
    m_localNames.resize(numLocalsStart);

    if (match(scanner::TokenType::Else)) {
        // if we are here, the condition passed, and we executed the `if` block
        // so get ready to jump past the `else` block(s)
        const auto trueJumpIndexes = emitJump();
        // and patch up the jump if the condition failed
        patchJump(falseJumpIndexes);

        if (match(scanner::TokenType::OpenBrace)) {
            const auto elseNumLocalsStart = m_localNames.size();

            if (!parseBlock("else block")) {
                return;
            }

            emitConstant(m_localNames.size() - elseNumLocalsStart);
            emitOp(runtime::Op::PopLocals, m_previous->line());
            m_localNames.resize(elseNumLocalsStart);
        } else if (match(scanner::TokenType::If)) {
            ifStatement();
        } else {
            errorAtPrevious("Expected '{' or 'if'");
            return;
        }

        // patch up the jump for skipping the `else` block(s)
        patchJump(trueJumpIndexes);
    } else {
        // no `else` block so no additional jumping, just patch up the false jump
        patchJump(falseJumpIndexes);
    }
}

auto Compiler::whileStatement() -> void
{
    if (m_contextStack.back() == Context::TopLevel) {
        errorAtPrevious("'while' not allowed at top level");
        return;
    }

    const auto function = m_vm->currentFunction();
    // need to jump here at the end of each iteration
    const auto constantIndex = function->numConstants();
    const auto opIndex = function->numOps();

    expression(false);
    // jump to after the loop when the condition is false
    const auto jumpIndexes = emitJump(JumpType::IfFalse, true);

    RETURN_IF_NO_MATCH(scanner::TokenType::OpenBrace, "Expected '{'");

    const auto numLocalsStart = m_localNames.size();

    if (!parseBlock("while loop")) {
        return;
    }

    // pop locals at the end of each iteration
    emitConstant(m_localNames.size() - numLocalsStart);
    emitOp(runtime::Op::PopLocals, m_previous->line());

    // jump back to re-evaluate the condition
    emitConstant(constantIndex);
    emitConstant(opIndex);
    emitOp(runtime::Op::Jump, m_previous->line());

    // patch in the jump for failing the condition
    patchJump(jumpIndexes);
}

auto Compiler::expression(bool canAssign) -> void
{
    // expressions can only start with a literal, unary op, or identifier
    if (scanner::isValidStartOfExpression(m_current->tokenType())) {
        logicOr(canAssign);
    } else {
        errorAtCurrent("Expected expression");
    }
}

auto Compiler::logicOr(bool canAssign) -> void
{
    logicAnd(canAssign);

    std::optional<JumpIndexes> jumpIndexes;
    if (check(scanner::TokenType::Or)) {
        jumpIndexes = emitJump(JumpType::IfTrue, false);
    }

    while (match(scanner::TokenType::Or)) {
        logicAnd(canAssign);
        emitOp(runtime::Op::LogicOr, m_previous->line());
    }

    if (jumpIndexes) {
        patchJump(*jumpIndexes);
    }
}

auto Compiler::logicAnd(bool canAssign) -> void
{
    bitwiseOr(canAssign);

    std::optional<JumpIndexes> jumpIndexes;
    if (check(scanner::TokenType::And)) {
        jumpIndexes = emitJump(JumpType::IfFalse, false);
    }

    while (match(scanner::TokenType::And)) {
        bitwiseOr(canAssign);
        emitOp(runtime::Op::LogicAnd, m_previous->line());
    }

    if (jumpIndexes) {
        patchJump(*jumpIndexes);
    }
}

auto Compiler::bitwiseOr(bool canAssign) -> void
{
    bitwiseXor(canAssign);

    while (match(scanner::TokenType::Pipe)) {
        bitwiseXor(canAssign);
        emitOp(runtime::Op::BitwiseOr, m_previous->line());
    }
}

auto Compiler::bitwiseXor(bool canAssign) -> void
{
    bitwiseAnd(canAssign);

    while (match(scanner::TokenType::Caret)) {
        bitwiseAnd(canAssign);
        emitOp(runtime::Op::BitwiseXor, m_previous->line());
    }
}

auto Compiler::bitwiseAnd(bool canAssign) -> void
{
    equality(canAssign);

    while (match(scanner::TokenType::Ampersand)) {
        equality(canAssign);
        emitOp(runtime::Op::BitwiseAnd, m_previous->line());
    }
}

auto Compiler::equality(bool canAssign) -> void
{
    comparison(canAssign);

    if (match(scanner::TokenType::EqualEqual)) {
        comparison(canAssign);
        emitOp(runtime::Op::Equal, m_previous->line());
    } else if (match(scanner::TokenType::NotEqual)) {
        comparison(canAssign);
        emitOp(runtime::Op::NotEqual, m_previous->line());
    }
}

auto Compiler::comparison(bool canAssign) -> void
{
    shift(canAssign);

    if (match(scanner::TokenType::Less)) {
        shift(canAssign);
        emitOp(runtime::Op::LessThan, m_previous->line());
    } else if (match(scanner::TokenType::LessEqual)) {
        shift(canAssign);
        emitOp(runtime::Op::LessEqual, m_previous->line());
    } else if (match(scanner::TokenType::Greater)) {
        shift(canAssign);
        emitOp(runtime::Op::GreaterThan, m_previous->line());
    } else if (match(scanner::TokenType::GreaterEqual)) {
        shift(canAssign);
        emitOp(runtime::Op::GreaterEqual, m_previous->line());
    }
}

auto Compiler::shift(bool canAssign) -> void
{
    term(canAssign);

    while (true) {
        if (match(scanner::TokenType::ShiftLeft)) {
            term(canAssign);
            emitOp(runtime::Op::LeftShift, m_previous->line());
        } else if (match(scanner::TokenType::ShiftRight)) {
            term(canAssign);
            emitOp(runtime::Op::RightShift, m_previous->line());
        } else {
            break;
        }
    }
}

auto Compiler::term(bool canAssign) -> void
{
    factor(canAssign);

    while (true) {
        if (match(scanner::TokenType::Plus)) {
            factor(canAssign);
            emitOp(runtime::Op::Addition, m_previous->line());
        } else if (match(scanner::TokenType::Minus)) {
            factor(canAssign);
            emitOp(runtime::Op::Subtraction, m_previous->line());
        } else {
            break;
        }
    }
}

auto Compiler::factor(bool canAssign) -> void
{
    unary(canAssign);

    while (true) {
        if (match(scanner::TokenType::Star)) {
            unary(canAssign);
            emitOp(runtime::Op::Multiply, m_previous->line());
        } else if (match(scanner::TokenType::Slash)) {
            unary(canAssign);
            emitOp(runtime::Op::Divide, m_previous->line());
        } else if (match(scanner::TokenType::Modulus)) {
            unary(canAssign);
            emitOp(runtime::Op::Modulus, m_previous->line());
        } else {
            break;
        }
    }
}

auto Compiler::unary(bool canAssign) -> void
{
    if (match(scanner::TokenType::Minus)) {
        const auto line = m_previous->line();
        unary(canAssign);
        emitOp(runtime::Op::Negate, line);
    } else if (match(scanner::TokenType::Tilde)) {
        const auto line = m_previous->line();
        unary(canAssign);
        emitOp(runtime::Op::BitwiseNot, line);
    } else if (match(scanner::TokenType::Exclamation)) {
        const auto line = m_previous->line();
        unary(canAssign);
        emitOp(runtime::Op::LogicNot, line);
    } else if (match(scanner::TokenType::Plus)) {
        auto line = m_previous->line();
        unary(canAssign);
        emitOp(runtime::Op::Plus, line);
    } else {
        call(canAssign);
    }
}

auto Compiler::call(bool canAssign) -> void
{
    primary(canAssign);

    while (true) {
        if (match(scanner::TokenType::OpenParen)) {
            if (const auto numArgs = parseCallArgs()) {
                emitConstant(*numArgs);
                emitOp(runtime::Op::Call, m_previous->line());
            }
        } else if (match(scanner::TokenType::Dot)) {
            RETURN_IF_NO_MATCH(scanner::TokenType::Identifier, "Expected identifier");
            emitConstant(m_previous->string());
            emitConstant(m_stringHasher(m_previous->string()));
            emitOp(runtime::Op::LoadMember, m_previous->line());

            if (match(scanner::TokenType::OpenParen)) {
                emitConstant(true);
                if (const auto numArgs = parseCallArgs()) {
                    emitConstant(*numArgs);
                    emitOp(runtime::Op::DotCall, m_previous->line());
                }
            }
        } else {
            break;
        }
    }
}

auto Compiler::primary(bool canAssign) -> void
{
    if (match(scanner::TokenType::False)) {
        emitConstant(false);
        emitOp(runtime::Op::LoadConstant, m_previous->line());
    } else if (match(scanner::TokenType::True)) {
        emitConstant(true);
        emitOp(runtime::Op::LoadConstant, m_previous->line());
    } else if (match(scanner::TokenType::Float)) {
        parseFloat();
    } else if (match(scanner::TokenType::Int)) {
        parseInt();
    } else if (match(scanner::TokenType::None)) {
        emitConstant(runtime::Value::none());
        emitOp(runtime::Op::LoadConstant, m_previous->line());
    } else if (match(scanner::TokenType::String)) {
        parseString();
    } else if (match(scanner::TokenType::OpenParen)) {
        expression(false);
        RETURN_IF_NO_MATCH(scanner::TokenType::CloseParen, "Expected ')'");
    } else if (match(scanner::TokenType::Identifier)) {
        identifier(canAssign);
    } else if (scanner::isTypeIdent(m_current->tokenType())) {
        advance();
        typeIdent();
    } else if (match(scanner::TokenType::TypeOf)) {
        typeOf();
    } else if (match(scanner::TokenType::Pipe)) {
        lambda();
    } else {
        errorAtCurrent("Invalid token at start of expression");
    }

    if (check(scanner::TokenType::Equal)) {
        // any assignment should have been handled in the above functions
        // if no assignment was allowed, no other tokens were consumed
        errorAtCurrent("Assignment is not allowed here");
    }
}

auto Compiler::identifier(bool canAssign) -> void
{
    auto identifier = m_previous->string();
    if (const auto findLocal = std::find_if(m_localNames.cbegin(), m_localNames.cend(), [&identifier](const LocalVariable& local) {
        return local.name == identifier;
    }); findLocal != m_localNames.cend()) {
        // a local variable that definitely exists
        const auto localIndex = std::distance(m_localNames.cbegin(), findLocal);

        if (match(scanner::TokenType::Equal)) {
            // trying to assign
            if (!canAssign) {
                errorAtPrevious("Assignment not allowed here");
                return;
            }

            if (findLocal->isFinal) {
                errorAtPrevious(fmt::format("'{}' is marked final", findLocal->name));
                return;
            }

            expression(false);
            emitConstant(localIndex);
            emitOp(runtime::Op::AssignLocal, m_previous->line());
        } else {
            // just loading the value
            emitConstant(localIndex);
            emitOp(runtime::Op::LoadLocal, m_previous->line());
        }
    } else {
        if (identifier.starts_with("__")) {
            // trying to call a native function
            nativeCall();
        } else if (check(scanner::TokenType::ColonColon)) {
            // qualifying a function with a namespace
            namespaceQualifiedCall();
        } else {
            // not a local, native call or a namespace qualification
            // so trying to call/load a function in the same namespace
            // resolve this at runtime
            emitConstant(m_filePathHash);
            emitConstant(m_stringHasher(identifier));
            emitConstant(std::move(identifier));
            emitOp(runtime::Op::LoadFunction, m_previous->line());
        }
    }
}

auto Compiler::nativeCall() -> void
{
    auto identifier = m_previous->text();

    if (const auto hash = m_vm->nativeFunctionHash(identifier)) {
        if (!m_stdFile) {
            errorAtPrevious("Calling native functions is only allowed in standard library files");
            return;
        }

        // we can just parse call args here...
        if (!match(scanner::TokenType::OpenParen)) {
            errorAtCurrent("Expected call for native function");
            return;
        }

        if (const auto numArgs = parseCallArgs()) {
            const auto arity = m_vm->nativeFunctionArity(*hash);
            if (*numArgs != arity) {
                errorAtPrevious(fmt::format("Expected {} arguments to native function {} but got {}", arity, identifier, *numArgs));
                return;
            }

            emitConstant(*hash);
            emitOp(runtime::Op::CallNative, m_previous->line());
        }
    } else {
        errorAtPrevious(fmt::format("Unrecognised native function '{}'", identifier));
        return;
    }
}

auto Compiler::namespaceQualifiedCall() -> void
{
    // currently, the only things you can have at the top level that you could try and load
    // are functions, and since we've already compiled the file you're looking in, we can verify this now
    // TODO: constants

    // so first, parse the rest of the namespace...
    const auto identifier = m_previous->string();

    std::filesystem::path namespaceFilePath;
    std::string namespaceText = identifier;

    advance(); // consume the first '::'

    if (m_importAliasLookup.contains(namespaceText)) {
        namespaceFilePath = m_importAliasLookup[namespaceText];
        advance(); // consume the function name
    } else {
        if (identifier == "std") {
            if (auto stdPath = getStdPath()) {
                namespaceFilePath.swap(*stdPath);
            } else {
                errorAtPrevious("The environment variable `POISE_STD_PATH` has not been set, cannot open std file");
                return;
            }
        } else {
            namespaceFilePath = m_filePath.parent_path() / identifier;
        }

        // current token is EITHER the next part of the namespace OR the function
        while (match(scanner::TokenType::Identifier)) {
            auto text = m_previous->text();

            if (match(scanner::TokenType::ColonColon)) {
                // continue namespace
                namespaceFilePath /= text;
                namespaceText += "::";
                namespaceText += text;
            } else {
                break;
            }
        }
    }

    auto verifyNamespaceAndFunction =
        [this](std::string_view functionName, std::string_view namespaceText, usize namespaceHash) -> bool {
            const auto namespaceManager = m_vm->namespaceManager();
            // load the function
            if (!namespaceManager->namespaceHasImportedNamespace(m_filePathHash, namespaceHash)) {
                errorAtPrevious(fmt::format("Namespace '{}' not imported", namespaceText));
                return false;
            }

            if (auto function = namespaceManager->namespaceFunction(namespaceHash, functionName)) {
                if (function->exported()) {
                    return true;
                } else {
                    errorAtPrevious(fmt::format("Function '{}' is not exported", functionName));
                    return false;
                }
            } else {
                errorAtPrevious(fmt::format("Function '{}' not found in namespace '{}'", functionName, namespaceText));
                return false;
            }
        };

    auto functionName = m_previous->string();
    const auto namespaceManager = m_vm->namespaceManager();

    if (match(scanner::TokenType::OpenParen)) {
        // function call
        // consume semicolon
        if (!namespaceFilePath.has_extension()) {
            namespaceFilePath += ".poise";
        }

        const auto namespaceHash = namespaceManager->namespaceHash(namespaceFilePath);
        if (!verifyNamespaceAndFunction(functionName, namespaceText, namespaceHash)) {
            return;
        }

        emitConstant(namespaceHash);
        emitConstant(m_stringHasher(functionName));
        emitConstant(functionName);
        emitOp(runtime::Op::LoadFunction, m_previous->line());

        if (const auto numArgs = parseCallArgs()) {
            const auto function = namespaceManager->namespaceFunction(namespaceHash, functionName);
            if (*numArgs != function->arity()) {
                errorAtPrevious(fmt::format("Expected {} args to '{}::{}()' but got {}", function->arity(), namespaceText, functionName, *numArgs));
                return;
            }

            emitConstant(*numArgs);
            emitOp(runtime::Op::Call, m_previous->line());
        }
    } else {
        if (!namespaceFilePath.has_extension()) {
            namespaceFilePath += ".poise";
        }

        const auto namespaceHash = namespaceManager->namespaceHash(namespaceFilePath);
        if (!verifyNamespaceAndFunction(functionName, namespaceText, namespaceHash)) {
            return;
        }

        emitConstant(namespaceHash);
        emitConstant(m_stringHasher(functionName));
        emitConstant(std::move(functionName));
        emitOp(runtime::Op::LoadFunction, m_previous->line());
    }
}

auto Compiler::typeIdent() -> void
{
    const auto tokenType = m_previous->tokenType();

    if (match(scanner::TokenType::OpenParen)) {
        // constructing an instance of the type
        if (const auto numArgs = parseCallArgs()) {
            if (*numArgs > 1) {
                // TODO: this will be different for collections
                errorAtPrevious(fmt::format("'{}' can only be constructed from a single argument but was given {}", tokenType, *numArgs));
                return;
            }

            emitConstant(static_cast<u8>(tokenType));
            emitConstant(*numArgs);
            emitOp(runtime::Op::ConstructBuiltin, m_previous->line());
        }
    } else {
        // just loading the type itself
        emitConstant(static_cast<u8>(tokenType));
        emitOp(runtime::Op::LoadType, m_previous->line());
    }
}

auto Compiler::typeOf() -> void
{
    RETURN_IF_NO_MATCH(scanner::TokenType::OpenParen, "Expected '('");
    expression(false);
    emitOp(runtime::Op::TypeOf, m_previous->line());
    RETURN_IF_NO_MATCH(scanner::TokenType::CloseParen, "Expected ')");
}

auto Compiler::lambda() -> void
{
    std::vector<usize> captureIndexes;
    std::vector<LocalVariable> captures;

    while (!match(scanner::TokenType::Pipe)) {
        if (match(scanner::TokenType::Identifier)) {
            if (captures.size() == std::numeric_limits<u8>::max()) {
                errorAtCurrent("Maximum amount of captures exceeded");
                return;
            }

            const auto text = m_previous->text();
            if (const auto localIt = std::find_if(m_localNames.cbegin(), m_localNames.cend(), [text](const LocalVariable& local) {
                return local.name == text;
            }); localIt != m_localNames.cend()) {
                if (std::find_if(captures.cbegin(), captures.cend(), [&text](const LocalVariable& local) {
                    return local.name == text;
                }) != captures.cend()) {
                    errorAtPrevious(fmt::format("Local variable '{}' has already been captured", text));
                    return;
                }

                const auto index = std::distance(m_localNames.cbegin(), localIt);
                captures.push_back(*localIt);
                captureIndexes.push_back(static_cast<usize>(index));

                // trailing commas are allowed but all arguments must be comma separated
                // so here, if the next token is not a comma or a pipe, it's invalid
                if (!check(scanner::TokenType::Pipe) && !check(scanner::TokenType::Comma)) {
                    errorAtCurrent("Expected ',' or '|'");
                    break;
                }

                if (check(scanner::TokenType::Comma)) {
                    advance();
                }
            } else {
                errorAtPrevious(fmt::format("No local variable named '{}' to capture", text));
            }
        } else {
            errorAtCurrent("Expected identifier for capture");
        }
    }

    auto oldLocals = std::move(m_localNames);
    m_localNames = std::move(captures);

    std::optional<FunctionArgsParseResult> args;
    if (match(scanner::TokenType::OpenParen)) {
        args = parseFunctionArgs(true);
        if (!args) {
            return;
        }
    }

    const auto [numArgs, extensionFunctionType] = *args;

    RETURN_IF_NO_MATCH(scanner::TokenType::OpenBrace, "Expected '{");

    const auto prevFunction = m_vm->currentFunction();

    m_contextStack.push_back(Context::Function);

    auto lambdaName = fmt::format("{}_lambda{}", prevFunction->name(), prevFunction->numLambdas());
    auto lambda = runtime::Value::createObject<objects::PoiseFunction>(std::move(lambdaName), m_filePath, m_filePathHash, numArgs, false);
    auto functionPtr = lambda.object()->asFunction();

    m_vm->setCurrentFunction(functionPtr);

    for (auto i = 0_uz; i < m_localNames.size() - numArgs; i++) {
        emitConstant(i);
        emitOp(runtime::Op::LoadCapture, m_previous->line());
    }

    if (!parseBlock("lambda")) {
        return;
    }

    if (functionPtr->opList().empty() || functionPtr->opList().back().op != runtime::Op::Return) {
        // if no return statement, make sure we pop locals and implicitly return none
        emitConstant(m_localNames.size());
        emitOp(runtime::Op::PopLocals, m_previous->line());
        emitConstant(runtime::Value::none());
        emitOp(runtime::Op::LoadConstant, m_previous->line());
        emitOp(runtime::Op::Return, m_previous->line());
    }

#ifdef POISE_DEBUG
    functionPtr->printOps();
#endif

    m_localNames = std::move(oldLocals);
    m_contextStack.pop_back();

    m_vm->setCurrentFunction(prevFunction);
    prevFunction->lamdaAdded();

    emitConstant(std::move(lambda));
    emitOp(runtime::Op::LoadConstant, m_previous->line());
    for (const auto index : captureIndexes) {
        emitConstant(index);
        emitOp(runtime::Op::CaptureLocal, m_previous->line());
    }
}

static auto getEscapeCharacter(char c) -> std::optional<char>
{
    switch (c) {
        case 't':
            return '\t';
        case 'n':
            return '\n';
        case 'r':
            return '\r';
        case '"':
            return '"';
        case '\\':
            return '\\';
        default:
            return {};
    }
}

auto Compiler::parseString() -> void
{
    std::string result;
    const auto tokenText = m_previous->text();
    auto i = 1_uz;
    while (i < tokenText.length() - 1_uz) {
        if (tokenText[i] == '\\') {
            i++;

            if (i == tokenText.length() - 1_uz) {
                errorAtPrevious("Expected escape character but string terminated");
                return;
            }

            if (const auto escapeChar = getEscapeCharacter(tokenText[i])) {
                result.push_back(*escapeChar);
            } else {
                errorAtPrevious(fmt::format("Unrecognised escape character '{}'", tokenText[i]));
                return;
            }
        } else {
            result.push_back(tokenText[i]);
        }

        i++;
    }

    emitConstant(std::move(result));
    emitOp(runtime::Op::LoadConstant, m_previous->line());
}

auto Compiler::parseInt() -> void
{
    auto result{0_i64};
    const auto text = m_previous->text();
    const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.length(), result);

    if (ec == std::errc{}) {
        emitConstant(result);
        emitOp(runtime::Op::LoadConstant, m_previous->line());
    } else if (ec == std::errc::invalid_argument) {
        errorAtPrevious(fmt::format("Unable to parse integer '{}'", text));
        return;
    } else if (ec == std::errc::result_out_of_range) {
        errorAtPrevious(fmt::format("Integer out of range '{}'", text));
        return;
    }
}

auto Compiler::parseFloat() -> void
{
    const auto text = m_previous->string();

    try {
        const auto result = std::stod(text);
        emitConstant(result);
        emitOp(runtime::Op::LoadConstant, m_previous->line());
    } catch (const std::invalid_argument&) {
        errorAtPrevious(fmt::format("Unable to parse float '{}'", text));
    } catch (const std::out_of_range&) {
        errorAtPrevious(fmt::format("Float out of range '{}'", text));
    }
}

auto Compiler::parseCallArgs() -> std::optional<u8>
{
    auto numArgs = 0_u8;

    while (!match(scanner::TokenType::CloseParen)) {
        if (numArgs == std::numeric_limits<u8>::max()) {
            errorAtCurrent("Maximum function parameters of 255 exceeded");
            return {};
        }

        expression(false);
        numArgs++;

        // trailing commas are allowed but all arguments must be comma separated
        // so here, if the next token is not a comma or a close paren, it's invalid
        if (!check(scanner::TokenType::CloseParen) && !check(scanner::TokenType::Comma)) {
            errorAtCurrent("Expected ',' or ')'");
            return {};
        }

        if (check(scanner::TokenType::Comma)) {
            advance();
        }
    }

    return numArgs;
}

auto Compiler::parseFunctionArgs(bool isLambda) -> std::optional<FunctionArgsParseResult>
{
    auto hasThisArg = false;
    auto numArgs = 0_u8;
    std::optional<types::Type> extensionFunctionType;

    while (!match(scanner::TokenType::CloseParen)) {
        if (numArgs == std::numeric_limits<u8>::max()) {
            errorAtCurrent("Maximum function parameters of 255 exceeded");
            return {};
        }

        if (match(scanner::TokenType::This)) {
            if (numArgs > 0) {
                errorAtPrevious("'this' only allowed on first parameter");
                return {};
            }

            if (isLambda) {
                errorAtPrevious("Lambdas cannot be extension functions");
                return {};
            }

            hasThisArg = true;
        }

        const auto isFinal = match(scanner::TokenType::Final);

        if (hasThisArg) {
            // TODO: handle user defined classes
            if (!scanner::isTypeIdent(m_current->tokenType())) {
                errorAtCurrent("Expected type for extension function");
                return {};
            }

            advance();

            switch (m_previous->tokenType()) {
                case scanner::TokenType::BoolIdent:
                    extensionFunctionType = types::Type::Bool;
                    break;
                case scanner::TokenType::FloatIdent:
                    extensionFunctionType = types::Type::Float;
                    break;
                case scanner::TokenType::IntIdent:
                    extensionFunctionType = types::Type::Int;
                    break;
                case scanner::TokenType::NoneIdent:
                    extensionFunctionType = types::Type::None;
                    break;
                case scanner::TokenType::StringIdent:
                    extensionFunctionType = types::Type::String;
                    break;
                case scanner::TokenType::ExceptionIdent:
                    extensionFunctionType = types::Type::Exception;
                    break;
                case scanner::TokenType::FunctionIdent:
                    extensionFunctionType = types::Type::Function;
                    break;
                default:
                    POISE_UNREACHABLE();
                    break;
            }

            hasThisArg = false;
        }

        if (!match(scanner::TokenType::Identifier)) {
            errorAtCurrent("Expected identifier");
            break;
        }

        auto argName = m_previous->string();
        if (std::find_if(m_localNames.cbegin(), m_localNames.cend(), [&argName](const LocalVariable& local) {
            return local.name == argName;
        }) != m_localNames.cend()) {
            errorAtPrevious("Function argument with the same name already declared");
            return {};
        }

        m_localNames.push_back({std::move(argName), isFinal});
        numArgs++;

        // trailing commas are allowed but all arguments must be comma separated
        // so here, if the next token is not a comma or a close paren, it's invalid
        if (!check(scanner::TokenType::CloseParen) && !check(scanner::TokenType::Comma)) {
            errorAtCurrent("Expected ',' or ')'");
            return {};
        }

        if (check(scanner::TokenType::Comma)) {
            advance();
        }
    }

    return {{numArgs, extensionFunctionType}};
}

auto Compiler::parseNamespaceImport() -> std::optional<NamespaceParseResult>
{
    std::filesystem::path namespaceFilePath;
    std::string namespaceName = m_previous->string();
    auto isStdFile = false;

    if (m_previous->text() == "std") {
        if (auto stdPath = getStdPath()) {
            isStdFile = true;
            namespaceFilePath.swap(*stdPath);
        } else {
            errorAtPrevious("The environment variable `POISE_STD_PATH` has not been set, cannot open std file");
            return {};
        }
    } else {
        namespaceFilePath = m_filePath.parent_path() / m_previous->text();
    }

    if (match(scanner::TokenType::Semicolon)) {
        namespaceFilePath += ".poise";
    } else {
        while (match(scanner::TokenType::ColonColon)) {
            namespaceName += "::";

            RETURN_VALUE_IF_NO_MATCH(scanner::TokenType::Identifier, "Expected namespace", std::nullopt);

            auto text = m_previous->string();
            namespaceName += text;

            if (match(scanner::TokenType::Semicolon)) {
                namespaceFilePath /= text + ".poise";
                break;
            } else if (match(scanner::TokenType::As)) {
                namespaceFilePath /= text + ".poise";

                RETURN_VALUE_IF_NO_MATCH(scanner::TokenType::Identifier, "Expected alias for namespace", std::nullopt);

                auto alias = m_previous->string();
                if (m_importAliasLookup.contains(alias)) {
                    errorAtPrevious(fmt::format("Namespace alias '{}' already used", alias));
                    return {};
                }

                m_importAliasLookup[std::move(alias)] = namespaceFilePath;

                EXPECT_SEMICOLON_RETURN_VALUE(std::nullopt);
                break;
            } else {
                namespaceFilePath /= m_previous->text();
            }
        }
    }

    return {{std::move(namespaceFilePath), std::move(namespaceName), isStdFile}};
}

auto Compiler::parseBlock(std::string_view scopeType) -> bool
{
    while (!match(scanner::TokenType::CloseBrace)) {
        if (check(scanner::TokenType::EndOfFile)) {
            errorAtCurrent(fmt::format("Unterminated {}", scopeType));
            return false;
        }

        if (m_hadError) {
            return false;
        }

        declaration();
    }

    return true;
}

auto Compiler::errorAtCurrent(std::string_view message) -> void
{
    error(*m_current, message);
}

auto Compiler::errorAtPrevious(std::string_view message) -> void
{
    error(*m_previous, message);
}

auto Compiler::error(const scanner::Token& token, std::string_view message) -> void
{
    m_hadError = true;

    fmt::print(stderr, fmt::emphasis::bold | fmt::fg(fmt::color::red), "Compiler Error");

    if (token.tokenType() == scanner::TokenType::EndOfFile) {
        fmt::print(stderr, " at EOF: {}", message);
    } else {
        fmt::print(stderr, " at '{}': {}\n", token.text(), message);
    }

    fmt::print(stderr, "       --> {}:{}:{}\n", m_filePath.string(), token.line(), token.column());
    fmt::print(stderr, "        |\n");

    if (token.line() > 1_uz) {
        fmt::print(stderr, "{:>7} | {}\n", token.line() - 1_uz, m_scanner.getCodeAtLine(token.line() - 1_uz));
    }

    fmt::print(stderr, "{:>7} | {}\n", token.line(), m_scanner.getCodeAtLine(token.line()));
    fmt::print(stderr, "        | ");
    for (auto i = 1_uz; i < token.column(); i++) {
        fmt::print(stderr, " ");
    }

    for (auto i = 0_uz; i < token.length(); i++) {
        fmt::print(stderr, fmt::fg(fmt::color::red), "^");
    }

    if (token.line() < m_scanner.getNumLines()) {
        fmt::print(stderr, "\n{:>7} | {}\n", token.line() + 1_uz, m_scanner.getCodeAtLine(token.line() + 1_uz));
    }

    fmt::print(stderr, "        |\n");
}
}   // namespace poise::compiler
