//
// Created by Ryan Jeffares on 15/12/2023.
//

#include "Compiler.hpp"
#include "Compiler_Macros.hpp"
#include "../runtime/Types.hpp"

#include <limits>
#include <ranges>

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
    return emitJump(JumpType::Jump, false);
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
        case JumpType::Jump:
            emitOp(runtime::Op::Jump, m_previous->line());
            break;
    }

    const auto jumpConstantIndex = function->numConstants();
    emitConstant(0_uz);
    const auto jumpOpIndex = function->numConstants();
    emitConstant(0_uz);

    if (jumpType != JumpType::Jump) {
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
    return std::ranges::find_if(m_localNames, [localName] (const LocalVariable& local) -> bool {
        return local.name == localName;
    }) != m_localNames.end();
}

auto Compiler::findLocal(std::string_view localName) const noexcept -> std::optional<LocalVariable>
{
    if (const auto it = std::ranges::find_if(m_localNames, [localName] (const LocalVariable& local) -> bool {
        return local.name == localName;
    }); it != m_localNames.end()) {
        return *it;
    }

    return std::nullopt;
}

auto Compiler::indexOfLocal(std::string_view localName) const noexcept -> std::optional<usize>
{
    if (const auto it = std::ranges::find_if(m_localNames, [localName](const LocalVariable& local) -> bool {
            return local.name == localName;
    }); it != m_localNames.end()) {
        return static_cast<usize>(std::distance(m_localNames.begin(), it));
    }

    return std::nullopt;
}

auto Compiler::checkLastOp(runtime::Op op) const noexcept -> bool
{
    return !m_vm->currentFunction()->opList().empty() && m_vm->currentFunction()->opList().back().op == op;
}

auto Compiler::lastOpWasAssignment() const noexcept -> bool
{
    // TODO: add member assignmen
    return checkLastOp(runtime::Op::AssignLocal) || checkLastOp(runtime::Op::AssignIndex);
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

        if (checkLastOp(runtime::Op::Unpack)) {
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
    auto hasVariadicParams = false;
    auto numParams = 0_u8;
    std::vector <runtime::types::Type> extensionFunctionTypes;

    while (!match(scanner::TokenType::CloseParen)) {
        if (numParams == std::numeric_limits<u8>::max()) {
            errorAtCurrent("Maximum function parameters of 255 exceeded");
            return {};
        }

        if (hasVariadicParams) {
            errorAtCurrent("Variadic parameter must be the last function parameter");
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

            while (true) {
                advance();
                extensionFunctionTypes.push_back(static_cast<runtime::types::Type>(m_previous->tokenType()));

                if (check(scanner::TokenType::Identifier)) {
                    break;
                }

                if (match(scanner::TokenType::Pipe)) {
                    if (!scanner::isTypeIdent(m_current->tokenType())) {
                        errorAtCurrent("Expected type");
                        return {};
                    }
                }
            }

            hasThisArg = false;
        }

        if (!match(scanner::TokenType::Identifier)) {
            errorAtCurrent("Expected identifier");
            break;
        }

        auto argName = m_previous->string();
        if (std::ranges::find_if(m_localNames, [&argName] (const LocalVariable& local) -> bool {
            return local.name == argName;
        }) != m_localNames.end()) {
            errorAtPrevious("Function parameter with the same name already declared");
            return {};
        }

        m_localNames.push_back({std::move(argName), isFinal});
        numParams++;

        if (match(scanner::TokenType::DotDotDot)) {
            hasVariadicParams = true;
        }

        if (match(scanner::TokenType::Colon)) {
            parseTypeAnnotation();
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

    return {{numParams, hasVariadicParams, std::move(extensionFunctionTypes)}};
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

            const auto text = m_previous->string();
            namespaceName += text;

            if (match(scanner::TokenType::Semicolon)) {
                namespaceFilePath /= text + ".poise";
                break;
            }

            if (match(scanner::TokenType::As)) {
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
            }

            namespaceFilePath /= m_previous->text();
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

auto Compiler::parseTypeAnnotation() -> void
{
    if (!scanner::isTypeIdent(m_current->tokenType())) {
        errorAtCurrent("Expected type");
        return;
    }

    advance();

    const auto tokenType = m_previous->tokenType();
    const auto genericTypeCount = scanner::builtinGenericTypeCount(tokenType);
    const auto typeName = scanner::typeDisplayName(tokenType);

    if (match(scanner::TokenType::OpenSquareBracket)) {
        if (genericTypeCount == scanner::AllowedGenericTypeCount::None) {
            errorAtPrevious(fmt::format("{} is not generic", typeName));
            return;
        }

        switch (genericTypeCount) {
            case scanner::AllowedGenericTypeCount::None:
                POISE_UNREACHABLE();
                return;
            case scanner::AllowedGenericTypeCount::One: {
                parseTypeAnnotation();
                RETURN_IF_NO_MATCH(scanner::TokenType::CloseSquareBracket, fmt::format("Expected ']' because {} uses 1 generic type", typeName));
                break;
            }
            case scanner::AllowedGenericTypeCount::Two: {
                parseTypeAnnotation();
                RETURN_IF_NO_MATCH(scanner::TokenType::Comma, fmt::format("Expected ',' because {} uses 2 generic types", typeName));
                parseTypeAnnotation();
                RETURN_IF_NO_MATCH(scanner::TokenType::CloseSquareBracket, fmt::format("Expected ']' because {} uses 2 generic types", typeName));
                break;
            }
            case scanner::AllowedGenericTypeCount::Any: {
                parseTypeAnnotation();
                while (!match(scanner::TokenType::CloseSquareBracket)) {
                    RETURN_IF_NO_MATCH(scanner::TokenType::Comma, fmt::format("Expected ','"));
                    parseTypeAnnotation();
                }
                break;
            }
        }
    }

    if (match(scanner::TokenType::Pipe)) {
        parseTypeAnnotation();
    }
}
}   // namespace poise::compiler
