#include "Compiler.hpp"

namespace poise::compiler
{
    Compiler::Compiler(std::filesystem::path inFilePath)
        : m_scanner{std::move(inFilePath)}
    {

    }

    auto Compiler::compile() -> CompileResult
    {
        while (true) {
            m_previous = m_current;
            m_current = m_scanner.scanToken();

            m_current->print();

            if (m_current->tokenType() == scanner::TokenType::Error) {
                return CompileResult::ParseError;
            }

            if (m_current->tokenType() == scanner::TokenType::EndOfFile) {
                break;
            }

            declaration();
        }

        return CompileResult::Success;
    }

    auto Compiler::advance() -> void
    {
        m_previous = m_current;
        m_current = m_scanner.scanToken();

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
        if (m_current->tokenType() == scanner::TokenType::Func) {
            funcDeclaration();
        } else {
            statement();
        }
    }

    auto Compiler::statement() -> void
    {
        if (m_current->tokenType() == scanner::TokenType::PrintLn) {
            printLnStatement();
        } else {
            expressionStatement();
        }
    }

    auto Compiler::expressionStatement() -> void
    {
        expression();
    }
}
