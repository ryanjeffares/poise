//
// Created by Ryan Jeffares on 15/12/2023.
//

#include "Compiler.hpp"
#include "Compiler_Macros.hpp"

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

    if (m_vm->namespaceManager()->namespaceFunction(m_filePathHash, functionName) != nullptr) {
        errorAtPrevious("Function with the same name already defined in this scope");
        return;
    }

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
        runtime::types::typeValue(*extensionFunctionType).object()->asType()->addExtensionFunction(function);
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

    if (hasLocal(varName)) {
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
}   // namespace poise::compiler
