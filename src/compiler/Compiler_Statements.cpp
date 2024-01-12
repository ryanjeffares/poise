//
// Created by Ryan Jeffares on 15/12/2023.
//

#include "Compiler.hpp"
#include "Compiler_Macros.hpp"

namespace poise::compiler {
auto Compiler::statement(bool consumeSemicolon) -> void
{
    if (m_contextStack.back() == Context::TopLevel) {
        errorAtCurrent("Statements not allowed at top level");
        return;
    }

    if (match(scanner::TokenType::Print) || match(scanner::TokenType::PrintLn)
        || match(scanner::TokenType::EPrint) || match(scanner::TokenType::EPrintLn)) {
        const auto err = m_previous->tokenType() == scanner::TokenType::EPrint || m_previous->tokenType() == scanner::TokenType::EPrintLn;
        const auto newLine = m_previous->tokenType() == scanner::TokenType::PrintLn || m_previous->tokenType() == scanner::TokenType::EPrintLn;
        printStatement(err, newLine, consumeSemicolon);
    } else if (match(scanner::TokenType::Return)) {
        returnStatement(consumeSemicolon);
    } else if (match(scanner::TokenType::Throw)) {
        throwStatement(consumeSemicolon);
    } else if (match(scanner::TokenType::Try)) {
        tryStatement();
    } else if (match(scanner::TokenType::If)) {
        ifStatement();
    } else if (match(scanner::TokenType::While)) {
        whileStatement();
    } else if (match(scanner::TokenType::For)) {
        forStatement();
    } else if (match(scanner::TokenType::Break)) {
        breakStatement();
    } else {
        expressionStatement(consumeSemicolon);
    }
}

auto Compiler::expressionStatement(bool consumeSemicolon) -> void
{
    /*
        calling `expression()` in any other context means the result of that expression is being consumed
        but this function is called when you have a line that's just like

        ```
        some_void_function();
        ```

        so emit an extra `Pop` instruction to remove that unused result
        OR an assignment

        ```
        var my_var = "hello";
        my_var = "goodbye"; // here
        ```

        in that case, nothing to pop if the last emitted op was assign local
        TODO: will need to check for assigning members and indexing
    */

    call(true);

    if (!lastOpWasAssignment()) {
        emitOp(runtime::Op::Pop, m_previous->line());
    }

    if (consumeSemicolon) {
        EXPECT_SEMICOLON();
    }
}

auto Compiler::printStatement(bool err, bool newLine, bool consumeSemicolon) -> void
{
    RETURN_IF_NO_MATCH(scanner::TokenType::OpenParen, "Expected '(' after 'println'");

    expression(false, false);
    emitConstant(err);
    emitConstant(newLine);
    emitOp(runtime::Op::Print, m_previous->line());

    RETURN_IF_NO_MATCH(scanner::TokenType::CloseParen, "Expected ')' after 'println'");

    if (consumeSemicolon) {
        EXPECT_SEMICOLON();
    }
}

auto Compiler::returnStatement(bool consumeSemicolon) -> void
{
    if (match(scanner::TokenType::Semicolon)) {
        // emit none value to return if no value is returned
        emitConstant(runtime::Value::none());
        emitOp(runtime::Op::LoadConstant, m_previous->line());
    } else {
        // else the return value should be any expression
        expression(false, false);
    }

    // pop local variables
    emitConstant(0);
    emitOp(runtime::Op::PopLocals, m_previous->line());

    // expression above is still on the stack
    emitOp(runtime::Op::Return, m_previous->line());

    if (consumeSemicolon) {
        EXPECT_SEMICOLON();
    }
}

auto Compiler::throwStatement(bool consumeSemicolon) -> void
{
    if (m_contextStack.back() == Context::TopLevel) {
        errorAtPrevious("'throw' not allowed at top level");
        return;
    }

    expression(false, false);
    emitOp(runtime::Op::Throw, m_previous->line());

    if (consumeSemicolon) {
        EXPECT_SEMICOLON();
    }
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
    emitConstant(numLocalsStart);
    emitOp(runtime::Op::PopLocals, m_previous->line());
    emitOp(runtime::Op::ExitTry, m_previous->line());
    const auto jumpIndexes = emitJump();

    // this patching is in the case of an exception being thrown - need to pop locals, and then continue into the catch block
    const auto numConstants = function->numConstants();
    const auto numOps = function->numOps();
    function->setConstant(numConstants, jumpConstantIndex);
    function->setConstant(numOps, jumpOpIndex);

    emitConstant(numLocalsStart);
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

    emitConstant(numLocalsStart);
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

    expression(false, false);
    RETURN_IF_NO_MATCH(scanner::TokenType::OpenBrace, "Expected '{'");

    const auto numLocalsStart = m_localNames.size();

    // jump if the condition fails, otherwise continue on and pop the condition result
    const auto falseJumpIndexes = emitJump(JumpType::IfFalse, true);

    if (!parseBlock("if statement")) {
        return;
    }

    emitConstant(numLocalsStart);
    emitOp(runtime::Op::PopLocals, m_previous->line());
    m_localNames.resize(numLocalsStart);

    if (match(scanner::TokenType::Else)) {
        // if we are here, the condition passed, and we executed the `if` block
        // so get ready to jump past the `else` block(s)
        const auto trueJumpIndexes = emitJump();
        // and patch up the jump if the condition failed
        patchJump(falseJumpIndexes);

        if (match(scanner::TokenType::OpenBrace)) {
            if (!parseBlock("else block")) {
                return;
            }

            emitConstant(numLocalsStart);
            emitOp(runtime::Op::PopLocals, m_previous->line());
            m_localNames.resize(numLocalsStart);
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
    m_breakOpStack.emplace();

    const auto function = m_vm->currentFunction();
    // need to jump here at the end of each iteration
    const auto constantIndex = function->numConstants();
    const auto opIndex = function->numOps();

    expression(false, false);
    // jump to after the loop when the condition is false
    const auto jumpIndexes = emitJump(JumpType::IfFalse, true);

    RETURN_IF_NO_MATCH(scanner::TokenType::OpenBrace, "Expected '{'");

    const auto numLocalsStart = m_localNames.size();

    if (!parseBlock("while loop")) {
        return;
    }

    // pop locals at the end of each iteration
    emitConstant(numLocalsStart);
    m_localNames.resize(numLocalsStart);
    emitOp(runtime::Op::PopLocals, m_previous->line());

    // jump back to re-evaluate the condition
    emitConstant(constantIndex);
    emitConstant(opIndex);
    emitOp(runtime::Op::Jump, m_previous->line());

    // patch break statements
    const auto breakJumpIndexes = std::move(m_breakOpStack.top());
    m_breakOpStack.pop();
    for (auto jumpIdxes : breakJumpIndexes) {
        patchJump(jumpIdxes);
    }

    // pop locals if we broke
    emitConstant(numLocalsStart);
    emitOp(runtime::Op::PopLocals, m_previous->line());

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
    m_breakOpStack.emplace();

    RETURN_IF_NO_MATCH(scanner::TokenType::Identifier, "Expected identifier");

    const auto firstIteratorName = m_previous->text();
    if (hasLocal(firstIteratorName)) {
        errorAtPrevious("Local variable with the same name already declared");
        return;
    }

    const auto firstIteratorLocalIndex = m_localNames.size();
    emitConstant(runtime::Value::none());
    emitOp(runtime::Op::LoadConstant, m_previous->line());
    emitOp(runtime::Op::DeclareLocal, m_previous->line());
    m_localNames.push_back({m_previous->string(), false});

    std::optional<usize> secondIteratorLocalIndex;
    if (match(scanner::TokenType::Comma)) {
        RETURN_IF_NO_MATCH(scanner::TokenType::Identifier, "Expected identifier");
        const auto secondIteratorName = m_previous->text();
        if (hasLocal(secondIteratorName)) {
            errorAtPrevious("Local variable with the same name already declared");
            return;
        }
        secondIteratorLocalIndex = m_localNames.size();
        emitConstant(runtime::Value::none());
        emitOp(runtime::Op::LoadConstant, m_previous->line());
        emitOp(runtime::Op::DeclareLocal, m_previous->line());
        m_localNames.push_back({m_previous->string(), false});
    }

    // includes all iterators
    const auto numLocalsStart = m_localNames.size();

    RETURN_IF_NO_MATCH(scanner::TokenType::In, "Expected 'in'");

    expression(false, false);
    emitConstant(firstIteratorLocalIndex);
    emitConstant(secondIteratorLocalIndex ? *secondIteratorLocalIndex : 0_uz);
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

    emitConstant(firstIteratorLocalIndex);
    emitConstant(secondIteratorLocalIndex ? *secondIteratorLocalIndex : 0_uz);
    emitOp(runtime::Op::IncrementIterator, m_previous->line());

    // pop locals at the end of each iteration
    emitConstant(numLocalsStart);
    emitOp(runtime::Op::PopLocals, m_previous->line());
    m_localNames.resize(numLocalsStart);

    // jump back to check the iterator
    emitConstant(constantIndex);
    emitConstant(opIndex);
    emitOp(runtime::Op::Jump, m_previous->line());

    // patch breaks
    const auto breakJumpIndexes = std::move(m_breakOpStack.top());
    m_breakOpStack.pop();
    for (auto jumpIdxes : breakJumpIndexes) {
        patchJump(jumpIdxes);
    }

    emitOp(runtime::Op::PopIterator, m_previous->line());

    patchJump(jumpIndexes);

    // finally, pop the iterators that were made as locals
    emitConstant(numLocalsStart - (secondIteratorLocalIndex ? 2 : 1));
    emitOp(runtime::Op::PopLocals, m_previous->line());
    m_localNames.resize(m_localNames.size() - (secondIteratorLocalIndex ? 2_uz : 1_uz));

    m_contextStack.pop_back();
}

auto Compiler::breakStatement() -> void
{
    const auto loopIt = std::find_if(m_contextStack.cbegin(), m_contextStack.cend(), [] (Context context) {
        return context == Context::ForLoop || context == Context::WhileLoop;
    });

    if (loopIt == m_contextStack.cend()) {
        errorAtPrevious("'break' only allowed inside of loops");
        return;
    }

    const auto lambdaIt = std::find(m_contextStack.cbegin(), m_contextStack.cend(), Context::Lambda);
    if (lambdaIt != m_contextStack.cend() && lambdaIt > loopIt) {
        errorAtPrevious("'break' only allowed inside of loops");
        return;
    }
    
    m_breakOpStack.top().push_back(emitJump(JumpType::Break, false));

    EXPECT_SEMICOLON();
}
}   // namespace poise::compiler
