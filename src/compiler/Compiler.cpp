#include "Compiler.hpp"
#include "../runtime/Value.hpp"
#include "../objects/PoiseFunction.hpp"

#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/format.h>

#define RETURN_IF_NO_MATCH(tokenType, message)          \
    do {                                                \
        if (!match(tokenType)) {                        \
            errorAtCurrent(message);                    \
            return;                                     \
        }                                               \
    } while (false)                                     \

#define EXPECT_SEMICOLON() RETURN_IF_NO_MATCH(scanner::TokenType::Semicolon, "Expected ';'")

namespace poise::compiler
{
    Compiler::Compiler(runtime::Vm* vm, std::filesystem::path inFilePath)
        : m_filePath{inFilePath}
        , m_scanner{std::move(inFilePath)}
        , m_vm{vm}
    {

    }

    auto Compiler::compile() -> CompileResult
    {
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

        if (m_mainFunction) {
            emitConstant(std::string{"main"});
            emitOp(runtime::Op::LoadConstant, 0);
            emitOp(runtime::Op::Call, 0);
            emitOp(runtime::Op::Exit, m_scanner.getNumLines());
        } else {
            errorAtPrevious("No main function declared");
            return CompileResult::CompileError;
        }

        return CompileResult::Success;
    }

    auto Compiler::emitOp(runtime::Op op, std::size_t line) -> void
    {
        m_vm->emitOp(op, line);
    }

    auto Compiler::emitConstant(runtime::Value value) -> void
    {
        m_vm->emitConstant(std::move(value));
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
        if (match(scanner::TokenType::Func)) {
            funcDeclaration();
        } else {
            statement();
        }
    }

    auto Compiler::funcDeclaration() -> void
    {
        RETURN_IF_NO_MATCH(scanner::TokenType::Identifier, "Expected function name");
        auto functionName = m_previous->string();
        auto line = m_previous->line();

        RETURN_IF_NO_MATCH(scanner::TokenType::OpenParen, "Expected '(' after function name");
        RETURN_IF_NO_MATCH(scanner::TokenType::CloseParen, "Expected ')' after function arguments");
        RETURN_IF_NO_MATCH(scanner::TokenType::Colon, "Expected ':' after function signature");

        auto prevFunction = m_vm->getCurrentFunction();

        auto function = runtime::Value::createObject<objects::PoiseFunction>(std::move(functionName), std::uint8_t{0});
        m_vm->setCurrentFunction(function.object()->asFunction());

        while (!match(scanner::TokenType::End)) {
            if (check(scanner::TokenType::EndOfFile)) {
                errorAtCurrent("Unterminated function");
                return;
            }

            declaration();
        }

        // TODO: handle returns properly
        emitOp(runtime::Op::Return, m_previous->line());

        m_vm->setCurrentFunction(prevFunction);

        if (function.object()->asFunction()->name() == "main") {
            m_mainFunction = function;
        }

        emitConstant(std::move(function));
        emitOp(runtime::Op::DeclareFunction, line);
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
        // TODO: recursive descent
        if (match(scanner::TokenType::String)) {
            string();
        } else {
            errorAtCurrent("Invalid token at start of expression");
        }
    }

    static auto getEscapeCharacter(char c) -> std::optional<char>
    {
        switch (c) {
            case 't': return '\t';
            case 'n': return '\n';
            case 'r': return '\r';
            case '"': return '\"';
            case '\\': return '\\';
            default: return {};
        }
    }

    auto Compiler::string() -> void
    {
        std::string result;
        auto tokenText = m_previous->text();
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

    auto Compiler::errorAtCurrent(const std::string& message) -> void
    {
        error(*m_current, message);
    }

    auto Compiler::errorAtPrevious(const std::string& message) -> void
    {
        error(*m_previous, message);
    }

    auto Compiler::error(const scanner::Token& token, const std::string& message) -> void
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
