#include "Compiler.hpp"
#include "../runtime/Value.hpp"
#include "../objects/PoiseFunction.hpp"

#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/format.h>

#include <charconv>

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
        : m_scanner{inFilePath}
        , m_filePath{std::move(inFilePath)}
        , m_vm{vm}
    {

    }

    auto Compiler::compile() -> CompileResult
    {
        if (!std::filesystem::exists(m_filePath) || m_filePath.extension() != ".poise") {
            return CompileResult::FileError;
        }

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
            emitConstant(*m_mainFunction);
            emitOp(runtime::Op::LoadConstant, 0zu);
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

    auto Compiler::advance() -> void
    {
        m_previous = m_current;
        m_current = m_scanner.scanToken();

// #ifdef POISE_DEBUG
//         m_current->print();
// #endif

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

        auto function = runtime::Value::createObject<objects::PoiseFunction>(std::move(functionName), u8{0});
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
        logicOr();
    }

    auto Compiler::logicOr() -> void
    {
        while (match(scanner::TokenType::Or)) {
            logicAnd();
            emitOp(runtime::Op::LogicAnd, m_previous->line());
        }
    }
    
    auto Compiler::logicAnd() -> void
    {
        
    }
    
    auto Compiler::bitwiseOr() -> void
    {
        
    }
    
    auto Compiler::bitwiseXor() -> void
    {
        
    }
    
    auto Compiler::bitwiseAnd() -> void
    {
        
    }
    
    auto Compiler::equality() -> void
    {
        
    }
    
    auto Compiler::comparison() -> void
    {
        
    }
    
    auto Compiler::shift() -> void
    {
        
    }
    
    auto Compiler::term() -> void
    {
        
    }
    
    auto Compiler::factor() -> void
    {
        
    }
    
    auto Compiler::unary() -> void
    {
        
    }
    
    auto Compiler::exponent() -> void
    {
        
    }
    
    auto Compiler::primary() -> void
    {
        if (match(scanner::TokenType::False)) {
            emitConstant(false);
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
        } else if (match(scanner::TokenType::True)) {
            emitConstant(true);
            emitOp(runtime::Op::LoadConstant, m_previous->line());
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

    auto Compiler::parseString() -> void
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

    auto Compiler::parseInt() -> void
    {
        i64 result;
        auto text = m_previous->text();
        auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.length(), result);
        
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
        auto text = m_previous->string();

        try {
            auto result = std::stod(text);
            emitConstant(result);
            emitOp(runtime::Op::LoadConstant, m_previous->line());
        } catch (const std::invalid_argument&) {
            errorAtPrevious(fmt::format("Unable to parse float '{}'", text));
        } catch (const std::out_of_range&) {
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
