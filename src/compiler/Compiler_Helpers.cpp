//
// Created by Ryan Jeffares on 15/12/2023.
//

#include "Compiler.hpp"
#include "Compiler_Macros.hpp"

#include <limits>

namespace poise::compiler {
auto Compiler::emitOp(runtime::Op op, usize line) const noexcept -> void
{
    m_vm->emitOp(op, line);
}

auto Compiler::emitConstant(runtime::Value value) const noexcept -> void
{
    m_vm->emitConstant(std::move(value));
}

auto Compiler::emitJump() const noexcept -> JumpIndexes
{
    return emitJump(JumpType::None, false);
}

auto Compiler::emitJump(JumpType jumpType, bool emitPop) const noexcept -> JumpIndexes
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

auto Compiler::patchJump(JumpIndexes jumpIndexes) const noexcept -> void
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

auto Compiler::check(scanner::TokenType expected) const noexcept -> bool
{
    return m_current && m_current->tokenType() == expected;
}

auto Compiler::hasLocal(std::string_view localName) const noexcept -> bool
{
    return std::find_if(m_localNames.cbegin(), m_localNames.cend(), [localName](const LocalVariable& local) {
        return local.name == localName;
    }) != m_localNames.cend();
}

auto Compiler::findLocal(std::string_view localName) const noexcept -> std::optional<LocalVariable>
{
    if (const auto it = std::find_if(m_localNames.cbegin(), m_localNames.cend(), [localName](const LocalVariable& local) {
        return local.name == localName;
    }); it != m_localNames.end()) {
        return *it;
    } else{
        return std::nullopt;
    }
}

auto Compiler::indexOfLocal(std::string_view localName) const noexcept -> std::optional<usize>
{
    if (const auto it = std::find_if(m_localNames.cbegin(), m_localNames.cend(), [localName](const LocalVariable& local) {
            return local.name == localName;
        }); it != m_localNames.end()) {
        return static_cast<usize>(std::distance(m_localNames.cbegin(), it));
    } else{
        return std::nullopt;
    }
}

auto Compiler::parseCallArgs(scanner::TokenType sentinel) -> std::optional<CallArgsParseResult>
{
    auto numArgs = 0_u8;
    auto hasUnpack = false;

    while (!match(sentinel)) {
        if (numArgs == std::numeric_limits<u8>::max()) {
            errorAtCurrent("Maximum function arguments of 255 exceeded");
            return {};
        }

        if (hasUnpack) {
            errorAtCurrent("Unpacking must be the last argument");
            return {};
        }

        expression(false, true);
        numArgs++;

        if (!m_vm->currentFunction()->opList().empty() && m_vm->currentFunction()->opList().back().op == runtime::Op::Unpack) {
            hasUnpack = true;
        }

        // trailing commas are allowed but all arguments must be comma separated
        // so here, if the next token is not a comma or a close paren, it's invalid
        if (!check(sentinel) && !check(scanner::TokenType::Comma)) {
            errorAtCurrent("Expected ',' or ')'");
            return {};
        }

        if (check(scanner::TokenType::Comma)) {
            advance();
        }
    }

    return {{numArgs, hasUnpack}};
}

auto Compiler::parseFunctionParams(bool isLambda) -> std::optional<FunctionParamsParseResult>
{
    auto hasThisArg = false;
    auto hasPack = false;
    auto numParams = 0_u8;
    std::optional<runtime::types::Type> extensionFunctionType;

    while (!match(scanner::TokenType::CloseParen)) {
        if (numParams == std::numeric_limits<u8>::max()) {
            errorAtCurrent("Maximum function parameters of 255 exceeded");
            return {};
        }

        if (hasPack) {
            errorAtCurrent("Pack must be the last function parameter");
        }

        if (match(scanner::TokenType::This)) {
            if (numParams > 0) {
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
            extensionFunctionType = static_cast<runtime::types::Type>(m_previous->tokenType());
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
            errorAtPrevious("Function parameter with the same name already declared");
            return {};
        }

        m_localNames.push_back({std::move(argName), isFinal});
        numParams++;

        if (match(scanner::TokenType::DotDotDot)) {
            hasPack = true;
        }

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

    return {{numParams, hasPack, extensionFunctionType}};
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

auto Compiler::parseAssignment(std::optional<usize> localIndex) -> void
{
    const auto function = m_vm->currentFunction();
    std::optional<usize> jumpConstantIndex, jumpOpIndex;

    if (match(scanner::TokenType::Try)) {
        jumpConstantIndex = function->numConstants();
        emitConstant(0_uz);
        jumpOpIndex = function->numConstants();
        emitConstant(0_uz);
        emitOp(runtime::Op::EnterTry, m_previous->line());
    }

    expression(false, false);

    if (jumpConstantIndex) {
        emitOp(runtime::Op::ExitTry, m_previous->line());
        const auto numConstants = function->numConstants();
        const auto numOps = function->numOps();
        function->setConstant(numConstants, *jumpConstantIndex);
        function->setConstant(numOps, *jumpOpIndex);
    }

    if (localIndex) {
        emitConstant(*localIndex);
        emitOp(runtime::Op::AssignLocal, m_previous->line());
    } else {
        emitOp(runtime::Op::DeclareLocal, m_previous->line());
    }
}
}   // namespace poise::compiler
