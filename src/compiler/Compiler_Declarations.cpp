//
// Created by Ryan Jeffares on 15/12/2023.
//

#include "Compiler.hpp"
#include "Compiler_Macros.hpp"
#include "../objects/Struct.hpp"
#include "../objects/Type.hpp"

namespace poise::compiler {
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
    } else if (match(scanner::TokenType::Const)) {
        constDeclaration(false);
    } else if (match(scanner::TokenType::Struct)) {
        structDeclaration(false);
    } else if (match(scanner::TokenType::Export)) {
        if (match(scanner::TokenType::Func)) {
            funcDeclaration(true);
        } else if (match(scanner::TokenType::Const)) {
            constDeclaration(true);
        } else if (match(scanner::TokenType::Struct)) {
            structDeclaration(true);
        } else {
            errorAtCurrent("Expected function");
        }
    } else {
        statement(true);
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

    if (!match(scanner::TokenType::Identifier) && !match(scanner::TokenType::DotDot)) {
        errorAtCurrent("Expected namespace or '..'");
        return;
    }

    const auto namespaceParseRes = parseNamespaceImport();
    if (!namespaceParseRes) {
        return;
    }

    for (const auto& [path, name, isStdFile] : *namespaceParseRes) {
        if (!std::filesystem::exists(path)) {
            errorAtPrevious(fmt::format("Cannot open file {}", path.string()));
            return;
        }

        if (m_vm->namespaceManager()->addNamespace(path, name, m_filePathHash)) {
            Compiler importCompiler{false, isStdFile, m_vm, path};
            if (importCompiler.compile() != CompileResult::Success)  {
                // set the error flag here, so we stop compiling
                // but no need to report - the import compiler already did this
                m_hadError = true;
            }
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

    if (!checkNameCollisions(functionName)) {
        return;
    }

    if (functionName.starts_with("__")) {
        errorAtPrevious("Function names may not start with '__' as this is reserved for the standard library");
        return;
    }

    RETURN_IF_NO_MATCH(scanner::TokenType::OpenParen, "Expected '(' after function name");
    const auto params = parseFunctionParams(false);
    if (!params) {
        return;
    }

    const auto& [numParams, hasVariadicParams, extensionFunctionTypes] = *params;

    if (match(scanner::TokenType::Colon)) {
        parseTypeAnnotation();
    }

    const auto isMainFunction = m_mainFile && functionName == "main";

    auto function = runtime::Value::createObjectUntracked<objects::Function>(
        std::move(functionName),
        m_filePath.string(),
        m_filePathHash,
        numParams,
        isExported || isMainFunction,
        hasVariadicParams
    );

    auto functionPtr = function.object()->asFunction();
    m_vm->setCurrentFunction(functionPtr);

    if (match(scanner::TokenType::OpenBrace)) {
        if (!parseBlock("function")) {
            return;
        }
    } else if (match(scanner::TokenType::Arrow)) {
        if (scanner::isValidStartOfExpression(m_current->tokenType())) {
            expression(false, false);
            emitConstant(0);
            if (lastOpWasAssignment()) {
                emitConstant(runtime::Value::none());
                emitOp(runtime::Op::LoadConstant, m_previous->line());
            }
            emitOp(runtime::Op::PopLocals, m_previous->line());
            emitOp(runtime::Op::Return, m_previous->line());
            EXPECT_SEMICOLON();
        } else {
            statement(true);
        }
    } else {
        errorAtCurrent("Expected '{' or '=>'");
        return;
    }

    if (!checkLastOp(runtime::Op::Return)) {
        // if no return statement, make sure we pop locals and implicitly return none
        emitConstant(0);
        emitOp(runtime::Op::PopLocals, m_previous->line());
        emitConstant(runtime::Value::none());
        emitOp(runtime::Op::LoadConstant, m_previous->line());
        emitOp(runtime::Op::Return, m_previous->line());
    }

    m_vm->setCurrentFunction(nullptr);

    if (isMainFunction) {
        m_mainFunction = function;
    }

#ifdef POISE_DEBUG
    functionPtr->printOps();
#endif

    if (!extensionFunctionTypes.empty()) {
        for (const auto type : extensionFunctionTypes) {
            m_vm->typeValue(type).object()->asType()->addExtensionFunction(function);
        }
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

    auto verifyVarName = [this] (std::string_view varName) -> bool {
        if (varName.starts_with("__")) {
            errorAtPrevious("Variable names may not start with '__' as this is reserved for the standard library");
            return false;
        }

        if (hasLocal(varName)) {
            errorAtPrevious("Local variable with the same name already declared");
            return false;
        }
        
        if (m_vm->namespaceManager()->hasConstant(m_filePathHash, varName)) {
            errorAtPrevious("Constant with the same name already declared in this namespace");
            return false;
        }

        return true;
    };

    const auto numDeclarationsBefore = m_localNames.size();

    if (!verifyVarName(m_previous->text())) {
        return;
    }

    std::vector varNames{m_previous->string()};
    m_localNames.push_back({m_previous->string(), isFinal});
    
    if (match(scanner::TokenType::Colon)) {
        parseTypeAnnotation();
    }

    while (match(scanner::TokenType::Comma)) {
        RETURN_IF_NO_MATCH(scanner::TokenType::Identifier, "Expected identifier");

        if (!verifyVarName(m_previous->text())) {
            return;
        }

        m_localNames.push_back({m_previous->string(), isFinal});
    }

    const auto numDeclarations = m_localNames.size() - numDeclarationsBefore;

    if (match(scanner::TokenType::Equal)) {
        if (match(scanner::TokenType::DotDotDot)) {
            unpack();
            emitConstant(numDeclarations);
            emitOp(runtime::Op::DeclareMultipleLocals, m_previous->line());
        } else {
            for (auto i = 0_uz; i < numDeclarations; i++) {
                expression(false, false);
                emitOp(runtime::Op::DeclareLocal, m_previous->line());

                if (i < numDeclarations - 1_uz && !match(scanner::TokenType::Comma)) {
                    errorAtCurrent("Expected ','");
                }
            }
        }
    } else {
        if (isFinal) {
            errorAtCurrent("Expected assignment after 'final'");
            return;
        }

        for (auto i = 0_uz; i < numDeclarations; i++) {
            emitConstant(runtime::Value::none());
            emitOp(runtime::Op::LoadConstant, m_previous->line());
            emitOp(runtime::Op::DeclareLocal, m_previous->line());
        }
    }

    EXPECT_SEMICOLON();
}

auto Compiler::constDeclaration(bool isExported) -> void
{
    RETURN_IF_NO_MATCH(scanner::TokenType::Identifier, "Expected identifier");

    auto constantName = m_previous->string();

    if (!checkNameCollisions(constantName)) {
        return;
    }

    if (match(scanner::TokenType::Colon)) {
        parseTypeAnnotation();
    }

    RETURN_IF_NO_MATCH(scanner::TokenType::Equal, "Expected assignment to 'const'");

    if (auto value = constantExpression()) {
        m_vm->namespaceManager()->addConstant(m_filePathHash, std::move(*value), std::move(constantName), isExported);
    }

    EXPECT_SEMICOLON();
}

auto Compiler::structDeclaration(bool isExported) -> void
{
    RETURN_IF_NO_MATCH(scanner::TokenType::Identifier, "Expected identifier");

    auto namespaceManager = m_vm->namespaceManager();
    auto structName = m_previous->string();

    if (!checkNameCollisions(structName)) {
        return;
    }

    RETURN_IF_NO_MATCH(scanner::TokenType::OpenBrace, "Expected '{'");

    std::vector<objects::Struct::MemberVariable> memberVariables;

    while (!match(scanner::TokenType::CloseBrace)) {
        RETURN_IF_NO_MATCH(scanner::TokenType::Identifier, "Expected member variable");
        auto memberName = m_previous->string();
        const auto memberNameHash = m_stringHasher(memberName);

        if (match(scanner::TokenType::Colon)) {
            parseTypeAnnotation();
        }

        if (match(scanner::TokenType::Equal)) {
            if (auto value = constantExpression()) {
                memberVariables.emplace_back(objects::Struct::MemberVariable{
                    .name = std::move(memberName),
                    .nameHash = memberNameHash,
                    .value = std::move(*value),
                });
            } else {
                return;
            }
        } else {
            memberVariables.emplace_back(objects::Struct::MemberVariable{
                .name = std::move(memberName),
                .nameHash = memberNameHash
            });
        }

        EXPECT_SEMICOLON();
    }

    namespaceManager->addStructToNamespace(
        m_filePathHash,
        runtime::Value::createObjectUntracked<objects::Struct>(
            std::move(structName),
            isExported,
            std::move(memberVariables)
        )
    );
}
}   // namespace poise::compiler

