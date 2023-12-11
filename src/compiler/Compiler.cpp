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

#define EXPECT_SEMICOLON() RETURN_IF_NO_MATCH(scanner::TokenType::Semicolon, "Expected ';'")

namespace poise::compiler {
Compiler::Compiler(bool mainFile, runtime::Vm* vm, std::filesystem::path inFilePath)
    : m_mainFile{mainFile}
    , m_scanner{inFilePath}
    , m_filePath{std::move(inFilePath)}
    , m_vm{vm}
{

}

auto Compiler::compile() -> CompileResult
{
    if (!std::filesystem::exists(m_filePath) || m_filePath.extension() != ".poise") {
        return CompileResult::FileError;
    }

    if (m_mainFile) {
        m_vm->addNamespace(m_filePath, "entry");
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
            emitConstant(m_vm->namespaceHash(m_filePath));
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

    if (emitPop && jumpType != JumpType::None) {
        // this will get hit if we don't jump
        emitOp(runtime::Op::Pop, m_previous->line());
    }

    const auto jumpConstantIndex = function->numConstants();
    emitConstant(0_uz);
    const auto jumpOpIndex = function->numConstants();
    emitConstant(0_uz);

    return {jumpConstantIndex, jumpOpIndex, emitPop && jumpType != JumpType::None};
}

auto Compiler::patchJump(JumpIndexes jumpIndexes) noexcept -> void
{
    const auto function = m_vm->currentFunction();

    const auto numOps = function->numOps();
    const auto numConstants = function->numConstants();
    function->setConstant(numOps, jumpIndexes.opIndex);
    function->setConstant(numConstants, jumpIndexes.constantIndex);

    if (jumpIndexes.emitPop) {
        // gets hit if we did jump originally
        emitOp(runtime::Op::Pop, m_previous->line());
    }
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
        funcDeclaration();
    } else if (match(scanner::TokenType::Var)) {
        varDeclaration(false);
    } else if (match(scanner::TokenType::Final)) {
        varDeclaration(true);
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

    const auto namespaceParseRes = parseNamespace();
    if (!namespaceParseRes) {
        return;
    }

    const auto& [path, name] = *namespaceParseRes;

    if (!std::filesystem::exists(path)) {
        errorAtPrevious(fmt::format("Cannot open file {}", path.string()));
        return;
    }

    m_vm->addNamespace(path, name);

    m_importCompiler = std::make_unique<Compiler>(false, m_vm, path);
    const auto res = m_importCompiler->compile();
    m_importCompiler.reset();
    if (res != CompileResult::Success) {
        // set the error flag here, so we stop compiling
        // but no need to report - the import compiler already did this
        m_hadError = true;
    }
}

auto Compiler::funcDeclaration() -> void
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
    const auto numArgs = parseFunctionArgs();

    RETURN_IF_NO_MATCH(scanner::TokenType::OpenBrace, "Expected '{' after function signature");

    auto prevFunction = m_vm->currentFunction();

    auto function = runtime::Value::createObject<objects::PoiseFunction>(std::move(functionName), m_filePath.string(), numArgs);
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

    m_vm->addFunctionToNamespace(m_filePath, std::move(function));

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

    if (std::find_if(m_localNames.begin(), m_localNames.end(), [&varName](const LocalVariable& local) {
        return local.name == varName;
    }) != m_localNames.end()) {
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

        if (std::find_if(m_localNames.begin(), m_localNames.end(), [&text] (const LocalVariable& local) {
            return local.name == text;
        }) != m_localNames.end()) {
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

    while (match(scanner::TokenType::OpenParen)) {
        const auto numArgs = parseCallArgs();
        emitConstant(numArgs);
        emitOp(runtime::Op::Call, m_previous->line());
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
    const auto identifier = m_previous->text();
    const auto findLocal = std::find_if(m_localNames.begin(), m_localNames.end(),
        [&identifier](const LocalVariable& local) {
            return local.name == identifier;
        });

    if (findLocal == m_localNames.end()) {
        if (identifier.starts_with("__")) {
            // trying to call a native function
            // TODO: restrict this to standard library files
            if (const auto hash = m_vm->nativeFunctionHash(identifier)) {
                // I THINK we can just parse call args here...
                if (!match(scanner::TokenType::OpenParen)) {
                    errorAtCurrent("Expected call for native function");
                    return;
                }

                const auto numArgs = parseCallArgs();
                const auto arity = m_vm->nativeFunctionArity(*hash);
                if (numArgs != arity) {
                    errorAtPrevious(fmt::format("Expected {} arguments to native function {} but got {}", arity, identifier, numArgs));
                    return;
                }

                emitConstant(*hash);
                emitOp(runtime::Op::CallNative, m_previous->line());
            } else {
                errorAtPrevious(fmt::format("Unrecognised native function '{}'", identifier));
                return;
            }
        } else {
            if (check(scanner::TokenType::ColonColon)) {
                // qualifying a function with a namespace
                // currently, the only things you can have at the top level that you could try and load
                // are functions, and since we've already compiled the file you're looking in, we can verify this now
                // TODO: constants

                // so first, parse the rest of the namespace...
                std::filesystem::path namespaceFilePath;
                std::string namespaceText{identifier};

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

                advance(); // consume the first '::'

                auto verifyNamespaceAndFunction =
                    [this] (std::string_view functionName, std::string_view namespaceText, const std::filesystem::path& namespacePath) -> bool {
                        // load the function
                        if (!m_vm->hasNamespace(namespacePath)) {
                            errorAtPrevious(fmt::format("Namespace '{}' not imported", namespaceText));
                            return false;
                        }

                        if (m_vm->namespaceFunction(namespacePath, functionName) == nullptr) {
                            errorAtPrevious(fmt::format("Function '{}' not found in namespace '{}'", functionName, namespaceText));
                            return false;
                        }

                        return true;
                    };

                // current token is EITHER the next part of the namespace OR the function
                while (match(scanner::TokenType::Identifier)) {
                    auto text = m_previous->string();

                    if (check(scanner::TokenType::Semicolon)) { // semicolon will be consumed by expressionStatement()
                        namespaceFilePath += ".poise";

                        const auto namespaceHash = m_vm->namespaceHash(namespaceFilePath);
                        if (!verifyNamespaceAndFunction(text, namespaceText, namespaceFilePath)) {
                            return;
                        }

                        emitConstant(namespaceHash);
                        emitConstant(std::move(text));
                        emitOp(runtime::Op::LoadFunction, m_previous->line());

                        break;
                    } else if (match(scanner::TokenType::OpenParen)) {
                        // function call
                        // consume semicolon
                        namespaceFilePath += ".poise";

                        const auto namespaceHash = m_vm->namespaceHash(namespaceFilePath);
                        if (!verifyNamespaceAndFunction(text, namespaceText, namespaceFilePath)) {
                            return;
                        }

                        emitConstant(namespaceHash);
                        emitConstant(text);
                        emitOp(runtime::Op::LoadFunction, m_previous->line());

                        const auto function = m_vm->namespaceFunction(namespaceFilePath, text);
                        const auto numArgs = parseCallArgs();

                        if (numArgs != function->arity()) {
                            errorAtPrevious(fmt::format("Expected {} args to '{}::{}()' but got {}", function->arity(), namespaceText, text, numArgs));
                            return;
                        }

                        emitConstant(numArgs);
                        emitOp(runtime::Op::Call, m_previous->line());

                        break;
                    } else if (match(scanner::TokenType::ColonColon)) {
                        // continue namespace
                        namespaceFilePath /= text;
                        namespaceText += "::";
                        namespaceText += text;
                    } else {
                        errorAtCurrent("Expected call or ';'");
                        return;
                    }
                }
            } else {
                // not a local, native call or a namespace qualification
                // so trying to call/load a function in the same namespace
                // resolve this at runtime
                emitConstant(m_vm->namespaceHash(m_filePath));
                emitConstant(identifier);
                emitOp(runtime::Op::LoadFunction, m_previous->line());
            }
        }
    } else {
        // a local variable that definitely exists
        const auto localIndex = std::distance(m_localNames.begin(), findLocal);

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
    }
}

auto Compiler::typeIdent() -> void
{
    const auto tokenType = m_previous->tokenType();

    if (match(scanner::TokenType::OpenParen)) {
        // constructing an instance of the type
        const auto numArgs = parseCallArgs();

        if (numArgs > 1) {
            // TODO: this will be different for collections
            errorAtPrevious(fmt::format("'{}' can only be constructed from a single argument but was given {}", tokenType, numArgs));
            return;
        }

        emitConstant(static_cast<u8>(tokenType));
        emitConstant(numArgs);
        emitOp(runtime::Op::ConstructBuiltin, m_previous->line());
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
            const auto localIt = std::find_if(m_localNames.begin(), m_localNames.end(), [text](const LocalVariable& local) {
                return local.name == text;
            });

            if (localIt == m_localNames.end()) {
                errorAtPrevious(fmt::format("No local variable named '{}' to capture", text));
            }

            if (std::find_if(captures.begin(), captures.end(), [&text](const LocalVariable& local) {
                return local.name == text;
            }) != captures.end()) {
                errorAtPrevious(fmt::format("Local variable '{}' has already been captured", text));
                return;
            }

            const auto index = std::distance(m_localNames.begin(), localIt);
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
            errorAtCurrent("Expected identifier for capture");
        }
    }

    auto oldLocals = std::move(m_localNames);
    m_localNames = std::move(captures);

    auto numArgs = 0_u8;
    if (match(scanner::TokenType::OpenParen)) {
        numArgs = parseFunctionArgs();
    }

    RETURN_IF_NO_MATCH(scanner::TokenType::OpenBrace, "Expected '{");

    const auto prevFunction = m_vm->currentFunction();

    m_contextStack.push_back(Context::Function);

    auto lambdaName = fmt::format("{}_lambda{}", prevFunction->name(), prevFunction->numLambdas());
    auto lambda = runtime::Value::createObject<objects::PoiseFunction>(std::move(lambdaName), m_filePath, numArgs);
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

auto Compiler::parseCallArgs() -> u8
{
    auto numArgs = 0_u8;

    while (!match(scanner::TokenType::CloseParen)) {
        if (numArgs == std::numeric_limits<u8>::max()) {
            errorAtCurrent("Maximum function parameters of 255 exceeded");
            break;
        }

        expression(false);
        numArgs++;

        // trailing commas are allowed but all arguments must be comma separated
        // so here, if the next token is not a comma or a close paren, it's invalid
        if (!check(scanner::TokenType::CloseParen) && !check(scanner::TokenType::Comma)) {
            errorAtCurrent("Expected ',' or ')'");
            break;
        }

        if (check(scanner::TokenType::Comma)) {
            advance();
        }
    }

    return numArgs;
}

auto Compiler::parseFunctionArgs() -> u8
{
    auto numArgs = 0_u8;

    while (!match(scanner::TokenType::CloseParen)) {
        if (numArgs == std::numeric_limits<u8>::max()) {
            errorAtCurrent("Maximum function parameters of 255 exceeded");
            break;
        }

        auto isFinal = match(scanner::TokenType::Final);

        if (!match(scanner::TokenType::Identifier)) {
            errorAtCurrent("Expected identifier");
            break;
        }

        auto argName = m_previous->string();
        if (std::find_if(m_localNames.begin(), m_localNames.end(), [&argName](const LocalVariable& local) {
            return local.name == argName;
        }) != m_localNames.end()) {
            errorAtPrevious("Function argument with the same name already declared");
            break;
        }

        m_localNames.push_back({std::move(argName), isFinal});
        numArgs++;

        // trailing commas are allowed but all arguments must be comma separated
        // so here, if the next token is not a comma or a close paren, it's invalid
        if (!check(scanner::TokenType::CloseParen) && !check(scanner::TokenType::Comma)) {
            errorAtCurrent("Expected ',' or ')'");
            break;
        }

        if (check(scanner::TokenType::Comma)) {
            advance();
        }
    }

    return numArgs;
}

auto Compiler::parseNamespace() -> std::optional<NamespaceParseResult>
{
    std::filesystem::path namespaceFilePath;
    std::string namespaceName = m_previous->string();

    if (m_previous->text() == "std") {
        if (auto stdPath = getStdPath()) {
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

            if (!match(scanner::TokenType::Identifier)) {
                errorAtCurrent("Expected namespace");
                return {};
            }

            auto text = m_previous->string();
            namespaceName += text;

            if (match(scanner::TokenType::Semicolon)) {
                namespaceFilePath /= text + ".poise";
                break;
            } else {
                namespaceFilePath /= m_previous->text();
            }
        }
    }

    return {{std::move(namespaceFilePath), std::move(namespaceName)}};
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
