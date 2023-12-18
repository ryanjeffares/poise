#include "Compiler.hpp"

#include <fmt/color.h>
#include <fmt/core.h>

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
            emitConstant(false);
            emitConstant(false);
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
        fmt::print(stderr, "{:>7} | {}\n", token.line() - 1_uz, m_scanner.getCodeAtLine(m_filePath, token.line() - 1_uz));
    }

    fmt::print(stderr, "{:>7} | {}\n", token.line(), m_scanner.getCodeAtLine(m_filePath, token.line()));
    fmt::print(stderr, "        | ");
    for (auto i = 1_uz; i < token.column(); i++) {
        fmt::print(stderr, " ");
    }

    for (auto i = 0_uz; i < token.length(); i++) {
        fmt::print(stderr, fmt::fg(fmt::color::red), "^");
    }

    if (token.line() < m_scanner.getNumLines()) {
        fmt::print(stderr, "\n{:>7} | {}\n", token.line() + 1_uz, m_scanner.getCodeAtLine(m_filePath, token.line() + 1_uz));
    }

    fmt::print(stderr, "        |\n");
}
}   // namespace poise::compiler
