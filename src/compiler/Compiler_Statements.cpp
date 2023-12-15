//
// Created by Ryan Jeffares on 15/12/2023.
//

#include "Compiler.hpp"
#include "Compiler_Macros.hpp"

namespace poise::compiler {
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
    } else if (match(scanner::TokenType::For)) {
        forStatement();
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

    m_contextStack.push_back(Context::TryCatch);

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

        if (hasLocal(text)) {
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

    m_contextStack.pop_back();
}

auto Compiler::ifStatement() -> void
{
    if (m_contextStack.back() == Context::TopLevel) {
        errorAtPrevious("'if' not allowed at top level");
        return;
    }

    m_contextStack.push_back(Context::IfStatement);

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

    m_contextStack.pop_back();
}

auto Compiler::whileStatement() -> void
{
    if (m_contextStack.back() == Context::TopLevel) {
        errorAtPrevious("'while' not allowed at top level");
        return;
    }

    m_contextStack.push_back(Context::WhileLoop);

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

    m_contextStack.pop_back();
}

auto Compiler::forStatement() -> void
{
    if (m_contextStack.back() == Context::TopLevel) {
        errorAtPrevious("'for' not allowed at top level");
        return;
    }

    m_contextStack.push_back(Context::ForLoop);

    RETURN_IF_NO_MATCH(scanner::TokenType::Identifier, "Expected identifier");

    const auto text = m_previous->text();
    if (hasLocal(text)) {
        errorAtPrevious("Local variable with the same name already declared");
        return;
    }

    const auto iteratorLocalIndex = m_localNames.size();
    m_localNames.push_back({m_previous->string(), false});
    emitConstant(runtime::Value::none());
    emitOp(runtime::Op::LoadConstant, m_previous->line());
    emitOp(runtime::Op::DeclareLocal, m_previous->line());

    RETURN_IF_NO_MATCH(scanner::TokenType::In, "Expected 'in'");

    expression(false);
    emitConstant(iteratorLocalIndex);
    emitOp(runtime::Op::InitIterator, m_previous->line());

    const auto function = m_vm->currentFunction();
    // need to jump here at the end of each iteration
    const auto constantIndex = function->numConstants();
    const auto opIndex = function->numOps();

    // after InitIterator and IncrementIterator, the value of Iterator::isAtEnd() is put onto the stack
    const auto jumpIndexes = emitJump(JumpType::IfTrue, true);

    RETURN_IF_NO_MATCH(scanner::TokenType::OpenBrace, "Expected '{'");

    if (!parseBlock("for loop")) {
        return;
    }

    emitConstant(iteratorLocalIndex);
    emitOp(runtime::Op::IncrementIterator, m_previous->line());

    // jump back to check the iterator
    emitConstant(constantIndex);
    emitConstant(opIndex);
    emitOp(runtime::Op::Jump, m_previous->line());

    patchJump(jumpIndexes);

    m_contextStack.pop_back();
}
}   // namespace poise::compiler
