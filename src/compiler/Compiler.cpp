#include "Compiler.hpp"
#include "../runtime/Value.hpp"
#include "../objects/PoiseFunction.hpp"

#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/format.h>

#include <charconv>
#include <limits>

#define RETURN_IF_NO_MATCH(tokenType, message)          \
    do {                                                \
        if (!match(tokenType)) {                        \
            errorAtCurrent(message);                    \
            return;                                     \
        }                                               \
    } while (false)                                     \

#define EXPECT_SEMICOLON() RETURN_IF_NO_MATCH(scanner::TokenType::Semicolon, "Expected ';'")

namespace poise::compiler {
    Compiler::Compiler(runtime::Vm *vm, std::filesystem::path inFilePath)
            : m_scanner{ inFilePath }
            , m_filePath{ std::move(inFilePath) }
            , m_vm{ vm }
    {

    }

    auto Compiler::compile() -> CompileResult
    {
        if (!std::filesystem::exists(m_filePath) || m_filePath.extension() != ".poise") {
            return CompileResult::FileError;
        }

        m_contextStack.push_back(Context::TopLevel);

        while (true) {
            if (m_hadError) {
                break;
            }

            advance();

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

        if (m_mainFunction) {
            emitConstant(*m_mainFunction);
            emitOp(runtime::Op::LoadConstant, 0zu);
            emitConstant(0);
            emitOp(runtime::Op::Call, 0zu);
            emitOp(runtime::Op::Exit, m_scanner.getNumLines());
        } else {
            errorAtPrevious("No main function declared");
            return CompileResult::CompileError;
        }

        return CompileResult::Success;
    }

    auto Compiler::emitOp(runtime::Op op, usize line) -> void
    {
        m_vm->emitOp(op, line);
    }

    auto Compiler::emitConstant(runtime::Value value) -> void
    {
        m_vm->emitConstant(std::move(value));
    }

    auto Compiler::emitJump(bool jumpCondition) -> JumpIndexes
    {
        const auto function = m_vm->getCurrentFunction();
        emitOp(jumpCondition ? runtime::Op::JumpIfTrue : runtime::Op::JumpIfFalse, m_previous->line());

        const auto jumpConstantIndex = function->numConstants();
        emitConstant(0zu);
        const auto jumpOpIndex = function->numConstants();
        emitConstant(0zu);

        return { jumpConstantIndex, jumpOpIndex };
    }

    auto Compiler::patchJump(JumpIndexes jumpIndexes) -> void
    {
        const auto function = m_vm->getCurrentFunction();

        const auto numOps = function->numOps();
        const auto numConstants = function->numConstants();
        function->setConstant(numOps, jumpIndexes.opIndex);
        function->setConstant(numConstants, jumpIndexes.constantIndex);
    }

    auto Compiler::advance() -> void
    {
        m_previous = std::move(m_current);
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
        if (match(scanner::TokenType::Func)) {
            funcDeclaration();
        } else if (match(scanner::TokenType::Var)) {
            varDeclaration();
        } else if (match(scanner::TokenType::Final)) {
            finalDeclaration();
        } else {
            statement();
        }
    }

    auto Compiler::funcDeclaration() -> void
    {
        m_contextStack.push_back(Context::Function);

        RETURN_IF_NO_MATCH(scanner::TokenType::Identifier, "Expected function name");
        auto functionName = m_previous->string();
        auto line = m_previous->line();

        RETURN_IF_NO_MATCH(scanner::TokenType::OpenParen, "Expected '(' after function name");
        RETURN_IF_NO_MATCH(scanner::TokenType::CloseParen, "Expected ')' after function arguments");
        RETURN_IF_NO_MATCH(scanner::TokenType::Colon, "Expected ':' after function signature");

        auto prevFunction = m_vm->getCurrentFunction();

        auto function = runtime::Value::createObject<objects::PoiseFunction>(std::move(functionName), u8{ 0 });
        m_vm->setCurrentFunction(function.object()->asFunction());

        while (!match(scanner::TokenType::End)) {
            if (check(scanner::TokenType::EndOfFile)) {
                errorAtCurrent("Unterminated function");
                return;
            }

            declaration();
        }

        // TODO: handle returns properly
        emitConstant(m_localNames.size());
        emitOp(runtime::Op::PopLocals, m_previous->line());
        emitOp(runtime::Op::Return, m_previous->line());

        m_vm->setCurrentFunction(prevFunction);

        if (function.object()->asFunction()->name() == "main") {
            m_mainFunction = function;
        }

        function.object()->asFunction()->printOps();

        emitConstant(std::move(function));
        emitOp(runtime::Op::DeclareFunction, line);

        m_localNames.clear();
        m_contextStack.pop_back();
    }

    auto Compiler::varDeclaration() -> void
    {
        if (std::find(m_contextStack.begin(), m_contextStack.end(), Context::Function) == m_contextStack.end()) {
            errorAtPrevious("'var' only allowed inside a function");
            return;
        }

        RETURN_IF_NO_MATCH(scanner::TokenType::Identifier, "Expected 'var' name");

        auto varName = m_previous->string();
        if (std::find(m_localNames.begin(), m_localNames.end(), varName) != m_localNames.end()) {
            errorAtPrevious("Local variable with the same name already declared");
            return;
        }

        m_localNames.push_back(std::move(varName));

        if (match(scanner::TokenType::Equal)) {
            expression();
        } else {
            emitConstant(nullptr);
            emitOp(runtime::Op::LoadConstant, m_previous->line());
        }

        emitOp(runtime::Op::DeclareLocal, m_previous->line());

        EXPECT_SEMICOLON();
    }

    auto Compiler::finalDeclaration() -> void
    {
        if (std::find(m_contextStack.begin(), m_contextStack.end(), Context::Function) == m_contextStack.end()) {
            errorAtPrevious("'final' only allowed inside a function");
            return;
        }

        RETURN_IF_NO_MATCH(scanner::TokenType::Identifier, "Expected 'final' name");

        auto varName = m_previous->string();
        if (std::find(m_localNames.begin(), m_localNames.end(), varName) != m_localNames.end()) {
            errorAtPrevious("Local variable with the same name already declared");
            return;
        }

        m_localNames.push_back(std::move(varName));

        RETURN_IF_NO_MATCH(scanner::TokenType::Equal, "Expected assignment to 'final'");

        expression();
        emitOp(runtime::Op::DeclareLocal, m_previous->line());

        EXPECT_SEMICOLON();
    }

    auto Compiler::statement() -> void
    {
        if (match(scanner::TokenType::PrintLn)) {
            printLnStatement();
        } else {
            expressionStatement();
        }
    }

    auto Compiler::printLnStatement() -> void
    {
        RETURN_IF_NO_MATCH(scanner::TokenType::OpenParen, "Expected '(' after 'println'");

        expression();
        emitOp(runtime::Op::PrintLn, m_previous->line());

        RETURN_IF_NO_MATCH(scanner::TokenType::CloseParen, "Expected ')' after 'println'");
        EXPECT_SEMICOLON();
    }

    auto Compiler::expressionStatement() -> void
    {
        expression();
        EXPECT_SEMICOLON();
    }

    auto Compiler::expression() -> void
    {
        // expressions can only start with a literal, unary op, or identifier
        if (scanner::isLiteral(m_current->tokenType()) || scanner::isUnaryOp(m_current->tokenType()) ||
            m_current->tokenType() == scanner::TokenType::OpenParen) {
            logicOr();
        }
    }

    auto Compiler::logicOr() -> void
    {
        logicAnd();

        JumpIndexes jumpIndexes;
        auto needsJumpPatch = false;

        if (check(scanner::TokenType::Or)) {
            jumpIndexes = emitJump(true);
            needsJumpPatch = true;
        }

        while (match(scanner::TokenType::Or)) {
            logicAnd();
            emitOp(runtime::Op::LogicOr, m_previous->line());
        }

        if (needsJumpPatch) {
            patchJump(jumpIndexes);
        }
    }

    auto Compiler::logicAnd() -> void
    {
        bitwiseOr();

        JumpIndexes jumpIndexes;
        auto needsJumpPatch = false;

        if (check(scanner::TokenType::And)) {
            jumpIndexes = emitJump(false);
            needsJumpPatch = true;
        }

        while (match(scanner::TokenType::And)) {
            bitwiseOr();
            emitOp(runtime::Op::LogicAnd, m_previous->line());
        }

        if (needsJumpPatch) {
            patchJump(jumpIndexes);
        }
    }

    auto Compiler::bitwiseOr() -> void
    {
        bitwiseXor();

        while (match(scanner::TokenType::Pipe)) {
            bitwiseXor();
            emitOp(runtime::Op::BitwiseOr, m_previous->line());
        }
    }

    auto Compiler::bitwiseXor() -> void
    {
        bitwiseAnd();

        while (match(scanner::TokenType::Caret)) {
            bitwiseAnd();
            emitOp(runtime::Op::BitwiseXor, m_previous->line());
        }
    }

    auto Compiler::bitwiseAnd() -> void
    {
        equality();

        while (match(scanner::TokenType::Ampersand)) {
            equality();
            emitOp(runtime::Op::BitwiseAnd, m_previous->line());
        }
    }

    auto Compiler::equality() -> void
    {
        comparison();

        if (match(scanner::TokenType::EqualEqual)) {
            comparison();
            emitOp(runtime::Op::Equal, m_previous->line());
        } else if (match(scanner::TokenType::NotEqual)) {
            comparison();
            emitOp(runtime::Op::NotEqual, m_previous->line());
        }
    }

    auto Compiler::comparison() -> void
    {
        shift();

        if (match(scanner::TokenType::Less)) {
            shift();
            emitOp(runtime::Op::LessThan, m_previous->line());
        } else if (match(scanner::TokenType::LessEqual)) {
            shift();
            emitOp(runtime::Op::LessEqual, m_previous->line());
        } else if (match(scanner::TokenType::Greater)) {
            shift();
            emitOp(runtime::Op::GreaterThan, m_previous->line());
        } else if (match(scanner::TokenType::GreaterEqual)) {
            shift();
            emitOp(runtime::Op::GreaterEqual, m_previous->line());
        }
    }

    auto Compiler::shift() -> void
    {
        term();

        while (true) {
            if (match(scanner::TokenType::ShiftLeft)) {
                term();
                emitOp(runtime::Op::LeftShift, m_previous->line());
            } else if (match(scanner::TokenType::ShiftRight)) {
                term();
                emitOp(runtime::Op::RightShift, m_previous->line());
            } else {
                break;
            }
        }
    }

    auto Compiler::term() -> void
    {
        factor();

        while (true) {
            if (match(scanner::TokenType::Plus)) {
                factor();
                emitOp(runtime::Op::Addition, m_previous->line());
            } else if (match(scanner::TokenType::Minus)) {
                factor();
                emitOp(runtime::Op::Subtraction, m_previous->line());
            } else {
                break;
            }
        }
    }

    auto Compiler::factor() -> void
    {
        unary();

        while (true) {
            if (match(scanner::TokenType::Star)) {
                unary();
                emitOp(runtime::Op::Multiply, m_previous->line());
            } else if (match(scanner::TokenType::Slash)) {
                unary();
                emitOp(runtime::Op::Divide, m_previous->line());
            } else if (match(scanner::TokenType::Modulus)) {
                unary();
                emitOp(runtime::Op::Modulus, m_previous->line());
            } else {
                break;
            }
        }
    }

    auto Compiler::unary() -> void
    {
        if (match(scanner::TokenType::Minus)) {
            const auto line = m_previous->line();
            unary();
            emitOp(runtime::Op::Negate, line);
        } else if (match(scanner::TokenType::Tilde)) {
            const auto line = m_previous->line();
            unary();
            emitOp(runtime::Op::BitwiseNot, line);
        } else if (match(scanner::TokenType::Exclamation)) {
            const auto line = m_previous->line();
            unary();
            emitOp(runtime::Op::LogicNot, line);
        } else if (match(scanner::TokenType::Plus)) {
            auto line = m_previous->line();
            unary();
            emitOp(runtime::Op::Plus, line);
        } else {
            call();
        }
    }

    auto Compiler::call() -> void
    {
        primary();

        while (match(scanner::TokenType::OpenParen)) {
            u8 numArgs = 0;

            while (true) {
                if (match(scanner::TokenType::CloseParen)) {
                    break;
                }

                if (numArgs == std::numeric_limits<u8>::max()) {
                    errorAtCurrent("Maximum function parameters of 255 exceeded");
                    return;
                }

                expression();
                numArgs++;

                // trailing commas are allowed but all arguments must be comma separated
                // so here, if the next token is not a comma or a close paren, it's invalid
                if (!check(scanner::TokenType::CloseParen) && !check(scanner::TokenType::Comma)) {
                    errorAtCurrent("Expected ',' or '('");
                    return;
                }

                if (check(scanner::TokenType::Comma)) {
                    advance();
                }
            }

            emitConstant(numArgs);
            emitOp(runtime::Op::Call, m_previous->line());
        }
    }

    auto Compiler::primary() -> void
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
            emitConstant(std::nullptr_t{});
            emitOp(runtime::Op::LoadConstant, m_previous->line());
        } else if (match(scanner::TokenType::String)) {
            parseString();
        } else if (match(scanner::TokenType::OpenParen)) {
            expression();
            RETURN_IF_NO_MATCH(scanner::TokenType::CloseParen, "Expected ')'");
        } else if (match(scanner::TokenType::Identifier)) {
            identifier();
        } else {
            errorAtCurrent("Invalid token at start of expression");
        }
    }

    auto Compiler::identifier() -> void
    {
        const auto identifier = m_previous->text();
        const auto findLocal = std::find(m_localNames.begin(), m_localNames.end(), identifier);

        if (findLocal == m_localNames.end()) {
            emitConstant(identifier);
            emitOp(runtime::Op::LoadFunction, m_previous->line());
        } else {
            const auto localIndex = std::distance(m_localNames.begin(), findLocal);
            emitConstant(localIndex);
            emitOp(runtime::Op::LoadLocal, m_previous->line());
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
        auto i = 1zu;
        while (i < tokenText.length() - 1zu) {
            if (tokenText[i] == '\\') {
                i++;

                if (i == tokenText.length() - 1zu) {
                    errorAtPrevious("Expected escape character but string terminated");
                    return;
                }

                if (auto escapeChar = getEscapeCharacter(tokenText[i])) {
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
        i64 result;
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
        } catch (const std::invalid_argument &) {
            errorAtPrevious(fmt::format("Unable to parse float '{}'", text));
        } catch (const std::out_of_range &) {
            errorAtPrevious(fmt::format("Float out of range '{}'", text));
        }
    }

    auto Compiler::errorAtCurrent(std::string_view message) -> void
    {
        error(*m_current, message);
    }

    auto Compiler::errorAtPrevious(std::string_view message) -> void
    {
        error(*m_previous, message);
    }

    auto Compiler::error(const scanner::Token &token, std::string_view message) -> void
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

        if (token.line() > 1zu) {
            fmt::print(stderr, "{:>7} | {}\n", token.line() - 1zu, m_scanner.getCodeAtLine(token.line() - 1zu));
        }

        fmt::print(stderr, "{:>7} | {}\n", token.line(), m_scanner.getCodeAtLine(token.line()));
        fmt::print(stderr, "        | ");
        for (auto i = 1zu; i < token.column(); i++) {
            fmt::print(stderr, " ");
        }

        for (auto i = 0zu; i < token.length(); i++) {
            fmt::print(stderr, fmt::fg(fmt::color::red), "^");
        }

        if (token.line() < m_scanner.getNumLines()) {
            fmt::print(stderr, "\n{:>7} | {}\n", token.line() + 1zu, m_scanner.getCodeAtLine(token.line() + 1zu));
        }

        fmt::print(stderr, "        |\n");
    }
}
