//
// Created by Ryan Jeffares on 15/12/2023.
//

#include "Compiler.hpp"
#include "Compiler_Macros.hpp"
#include "../runtime/memory/StringInterner.hpp"

#include <charconv>
#include <version>

namespace poise::compiler {
auto Compiler::expression(bool canAssign, bool canUnpack) -> void
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

    // expressions can only start with a literal, unary op, or identifier
    if (scanner::isValidStartOfExpression(m_current->tokenType())) {
        range(canAssign);
    } else if (match(scanner::TokenType::DotDotDot)) {
        if (!canUnpack) {
            errorAtPrevious("Unpacking is not allowed here");
            return;
        }
        unpack();
    } else {
        errorAtCurrent("Expected expression");
    }

    if (jumpConstantIndex) {
        emitOp(runtime::Op::ExitTry, m_previous->line());
        const auto numConstants = function->numConstants();
        const auto numOps = function->numOps();
        function->setConstant(numConstants, *jumpConstantIndex);
        function->setConstant(numOps, *jumpOpIndex);
    }
}

auto Compiler::unpack() -> void
{
    expression(false, false);
    emitOp(runtime::Op::Unpack, m_previous->line());
}

auto Compiler::range(bool canAssign) -> void
{
    logicOr(canAssign);

    if (match(scanner::TokenType::DotDot) || match(scanner::TokenType::DotDotEqual)) {
        const auto inclusive = m_previous->tokenType() == scanner::TokenType::DotDotEqual;
        logicOr(canAssign);

        if (match(scanner::TokenType::By)) {
            expression(false, false);
        } else {
            emitConstant(1);
            emitOp(runtime::Op::LoadConstant, m_previous->line());
        }

        emitConstant(static_cast<u8>(runtime::types::Type::Range));
        emitConstant(3);
        emitConstant(false);
        emitConstant(inclusive);
        emitOp(runtime::Op::ConstructBuiltin, m_previous->line());

    }
}

auto Compiler::logicOr(bool canAssign) -> void
{
    logicAnd(canAssign);

    std::optional<JumpIndexes> jumpIndexes;
    if (check(scanner::TokenType::Or)) {
        jumpIndexes = emitJump(JumpType::IfTrue, false);
    }

    while (match(scanner::TokenType::Or)) {
        logicAnd(canAssign);
        emitOp(runtime::Op::LogicOr, m_previous->line());
    }

    if (jumpIndexes) {
        patchJump(*jumpIndexes);
    }
}

auto Compiler::logicAnd(bool canAssign) -> void
{
    bitwiseOr(canAssign);

    std::optional<JumpIndexes> jumpIndexes;
    if (check(scanner::TokenType::And)) {
        jumpIndexes = emitJump(JumpType::IfFalse, false);
    }

    while (match(scanner::TokenType::And)) {
        bitwiseOr(canAssign);
        emitOp(runtime::Op::LogicAnd, m_previous->line());
    }

    if (jumpIndexes) {
        patchJump(*jumpIndexes);
    }
}

auto Compiler::bitwiseOr(bool canAssign) -> void
{
    bitwiseXor(canAssign);

    while (match(scanner::TokenType::Pipe)) {
        bitwiseXor(canAssign);
        emitOp(runtime::Op::BitwiseOr, m_previous->line());
    }
}

auto Compiler::bitwiseXor(bool canAssign) -> void
{
    bitwiseAnd(canAssign);

    while (match(scanner::TokenType::Caret)) {
        bitwiseAnd(canAssign);
        emitOp(runtime::Op::BitwiseXor, m_previous->line());
    }
}

auto Compiler::bitwiseAnd(bool canAssign) -> void
{
    equality(canAssign);

    while (match(scanner::TokenType::Ampersand)) {
        equality(canAssign);
        emitOp(runtime::Op::BitwiseAnd, m_previous->line());
    }
}

auto Compiler::equality(bool canAssign) -> void
{
    comparison(canAssign);

    if (match(scanner::TokenType::EqualEqual)) {
        comparison(canAssign);
        emitOp(runtime::Op::Equal, m_previous->line());
    } else if (match(scanner::TokenType::NotEqual)) {
        comparison(canAssign);
        emitOp(runtime::Op::NotEqual, m_previous->line());
    }
}

auto Compiler::comparison(bool canAssign) -> void
{
    shift(canAssign);

    if (match(scanner::TokenType::Less)) {
        shift(canAssign);
        emitOp(runtime::Op::LessThan, m_previous->line());
    } else if (match(scanner::TokenType::LessEqual)) {
        shift(canAssign);
        emitOp(runtime::Op::LessEqual, m_previous->line());
    } else if (match(scanner::TokenType::Greater)) {
        shift(canAssign);
        emitOp(runtime::Op::GreaterThan, m_previous->line());
    } else if (match(scanner::TokenType::GreaterEqual)) {
        shift(canAssign);
        emitOp(runtime::Op::GreaterEqual, m_previous->line());
    }
}

auto Compiler::shift(bool canAssign) -> void
{
    term(canAssign);

    while (true) {
        if (match(scanner::TokenType::ShiftLeft)) {
            term(canAssign);
            emitOp(runtime::Op::LeftShift, m_previous->line());
        } else if (match(scanner::TokenType::ShiftRight)) {
            term(canAssign);
            emitOp(runtime::Op::RightShift, m_previous->line());
        } else {
            break;
        }
    }
}

auto Compiler::term(bool canAssign) -> void
{
    factor(canAssign);

    while (true) {
        if (match(scanner::TokenType::Plus)) {
            factor(canAssign);
            emitOp(runtime::Op::Addition, m_previous->line());
        } else if (match(scanner::TokenType::Minus)) {
            factor(canAssign);
            emitOp(runtime::Op::Subtraction, m_previous->line());
        } else {
            break;
        }
    }
}

auto Compiler::factor(bool canAssign) -> void
{
    unary(canAssign);

    while (true) {
        if (match(scanner::TokenType::Star)) {
            unary(canAssign);
            emitOp(runtime::Op::Multiply, m_previous->line());
        } else if (match(scanner::TokenType::Slash)) {
            unary(canAssign);
            emitOp(runtime::Op::Divide, m_previous->line());
        } else if (match(scanner::TokenType::Modulus)) {
            unary(canAssign);
            emitOp(runtime::Op::Modulus, m_previous->line());
        } else {
            break;
        }
    }
}

auto Compiler::unary(bool canAssign) -> void
{
    if (match(scanner::TokenType::Minus)) {
        const auto line = m_previous->line();
        unary(canAssign);
        emitOp(runtime::Op::Negate, line);
    } else if (match(scanner::TokenType::Tilde)) {
        const auto line = m_previous->line();
        unary(canAssign);
        emitOp(runtime::Op::BitwiseNot, line);
    } else if (match(scanner::TokenType::Exclamation)) {
        const auto line = m_previous->line();
        unary(canAssign);
        emitOp(runtime::Op::LogicNot, line);
    } else if (match(scanner::TokenType::Plus)) {
        const auto line = m_previous->line();
        unary(canAssign);
        emitOp(runtime::Op::Plus, line);
    } else {
        call(canAssign);
    }
}

auto Compiler::call(bool canAssign) -> void
{
    primary(canAssign);

    while (true) {
        if (match(scanner::TokenType::OpenParen)) {
            if (const auto args = parseCallArgs(scanner::TokenType::CloseParen)) {
                const auto [numArgs, hasUnpack] = *args;
                emitConstant(numArgs);
                emitConstant(hasUnpack);
                emitConstant(false);
                emitOp(runtime::Op::Call, m_previous->line());
            }
        } else if (match(scanner::TokenType::Dot)) {
            RETURN_IF_NO_MATCH(scanner::TokenType::Identifier, "Expected identifier");
            emitConstant(runtime::memory::internString(m_previous->string()));
            emitOp(runtime::Op::LoadMember, m_previous->line());

            if (match(scanner::TokenType::OpenParen)) {
                emitConstant(true); // flag to dictate whether to push the parent back on the stack since this is a dot call
                if (const auto args = parseCallArgs(scanner::TokenType::CloseParen)) {
                    const auto [numArgs, hasUnpack] = *args;
                    emitConstant(numArgs);
                    emitConstant(hasUnpack);
                    emitConstant(true);
                    emitOp(runtime::Op::Call, m_previous->line());
                }
            } else {
                emitConstant(false); // don't push parent back on to the stack
            }
        } else if (match(scanner::TokenType::OpenSquareBracket)) {
            expression(false, false);
            RETURN_IF_NO_MATCH(scanner::TokenType::CloseSquareBracket, "Expected ']'");

            if (match(scanner::TokenType::Equal)) {
                if (!canAssign) {
                    errorAtPrevious("Assignment is not allowed here");
                    return;
                }

                expression(false, false);
                emitOp(runtime::Op::AssignIndex, m_previous->line());
            } else {
                emitOp(runtime::Op::LoadIndex, m_previous->line());
            }
        } else {
            break;
        }
    }
}

auto Compiler::primary(bool canAssign) -> void
{
    if (match(scanner::TokenType::False)) {
        emitConstant(false);
        emitOp(runtime::Op::LoadConstant, m_previous->line());
    } else if (match(scanner::TokenType::True)) {
        emitConstant(true);
        emitOp(runtime::Op::LoadConstant, m_previous->line());
    } else if (match(scanner::TokenType::Float)) {
        if (const auto f = parseFloat()) {
            emitConstant(*f);
            emitOp(runtime::Op::LoadConstant, m_previous->line());
        }
    } else if (match(scanner::TokenType::Int)) {
        if (const auto i = parseInt()) {
            emitConstant(*i);
            emitOp(runtime::Op::LoadConstant, m_previous->line());
        }
    } else if (match(scanner::TokenType::None)) {
        emitConstant(runtime::Value::none());
        emitOp(runtime::Op::LoadConstant, m_previous->line());
    } else if (match(scanner::TokenType::String)) {
        if (auto s = parseString()) {
            emitConstant(std::move(*s));
            emitOp(runtime::Op::LoadConstant, m_previous->line());
        }
    } else if (match(scanner::TokenType::OpenParen)) {
        tupleOrGrouping();
    } else if (match(scanner::TokenType::Identifier)) {
        identifier(canAssign);
    } else if (scanner::isTypeIdent(m_current->tokenType())) {
        advance();
        typeIdent();
    } else if (match(scanner::TokenType::TypeOf)) {
        typeOf();
    } else if (match(scanner::TokenType::Pipe)) {
        lambda();
    } else if (match(scanner::TokenType::OpenSquareBracket)) {
        list();
    } else if (match(scanner::TokenType::OpenBrace)) {
        dict();
    } else {
        errorAtCurrent("Invalid token at start of expression");
    }

    if (check(scanner::TokenType::Equal)) {
        // any assignment should have been handled in the above functions
        // if no assignment was allowed, no other tokens were consumed
        errorAtCurrent("Assignment is not allowed here");
    }
}

auto Compiler::identifier(bool canAssign) -> void
{
    auto identifier = m_previous->string();
    if (const auto localIndex = indexOfLocal(identifier)) {
        // a local variable that definitely exists
        if (match(scanner::TokenType::Equal)) {
            // trying to assign
            if (!canAssign) {
                errorAtPrevious("Assignment not allowed here");
                return;
            }

            if (m_localNames[*localIndex].isFinal) {
                errorAtPrevious(fmt::format("'{}' is marked final", m_localNames[*localIndex].name));
                return;
            }

            expression(false, false);
            emitConstant(*localIndex);
            emitOp(runtime::Op::AssignLocal, m_previous->line());
        } else {
            // just loading the value
            emitConstant(*localIndex);
            emitOp(runtime::Op::LoadLocal, m_previous->line());
        }
    } else if (const auto constant = m_vm->namespaceManager()->getConstant(m_filePathHash, identifier)) {
        emitConstant(constant->value);
        emitOp(runtime::Op::LoadConstant, m_previous->line());
    } else {
        if (identifier.starts_with("__")) {
            // trying to call a native function
            nativeCall();
        } else if (check(scanner::TokenType::ColonColon)) {
            // qualifying a function with a namespace
            namespaceQualifiedCall();
        } else {
            // not a local, native call or a namespace qualification
            // so trying to call/load a function in the same namespace
            // resolve this at runtime
            emitConstant(m_filePathHash);
            emitConstant(runtime::memory::internString(std::move(identifier)));
            emitOp(runtime::Op::LoadFunction, m_previous->line());
        }
    }
}

auto Compiler::nativeCall() -> void
{
    auto identifier = m_previous->text();

    if (const auto hash = m_vm->nativeFunctionHash(identifier)) {
        if (!m_stdFile) {
            errorAtPrevious("Calling native functions is only allowed in standard library files");
            return;
        }

        // we can just parse call args here...
        if (!match(scanner::TokenType::OpenParen)) {
            errorAtCurrent("Expected call for native function");
            return;
        }

        if (const auto args = parseCallArgs(scanner::TokenType::CloseParen)) {
            const auto arity = m_vm->nativeFunctionArity(*hash);
            const auto [numArgs, hasUnpack] = *args;

            if (hasUnpack) {
                errorAtPrevious("Unpacking not allowed in native function calls");
                return;
            }

            if (numArgs != arity) {
                errorAtPrevious(fmt::format("Expected {} arguments to native function {} but got {}", arity, identifier, numArgs));
                return;
            }

            emitConstant(*hash);
            emitOp(runtime::Op::CallNative, m_previous->line());
        }
    } else {
        errorAtPrevious(fmt::format("Unrecognised native function '{}'", identifier));
    }
}

auto Compiler::namespaceQualifiedCall() -> void
{
    auto parseResult = parseNamespaceQualification();
    if (!parseResult) {
        return;
    }

    const auto& [namespaceText, namespaceHash] = *parseResult;
    const auto namespaceManager = m_vm->namespaceManager();

    if (!namespaceManager->namespaceHasImportedNamespace(m_filePathHash, namespaceHash)) {
        errorAtPrevious(fmt::format("Namespace '{}' not imported", namespaceText));
        return;
    }

    if (const auto constant = namespaceManager->getConstant(namespaceHash, m_previous->text())) {
        if (!constant->isExported) {
            errorAtPrevious(fmt::format("Constant '{}' in namespace '{}' is not exported", m_previous->text(), namespaceText));
            return;
        }

        emitConstant(constant->value);
        emitOp(runtime::Op::LoadConstant, m_previous->line());
    } else {
        emitConstant(namespaceHash);
        emitConstant(runtime::memory::internString(m_previous->string()));
        emitOp(runtime::Op::LoadFunction, m_previous->line());

        if (match(scanner::TokenType::OpenParen)) {
            if (const auto args = parseCallArgs(scanner::TokenType::CloseParen)) {
                const auto [numArgs, hasUnpack] = *args;
                emitConstant(numArgs);
                emitConstant(hasUnpack);
                emitConstant(false);
                emitOp(runtime::Op::Call, m_previous->line());
            }
        }
    }
}

auto Compiler::typeIdent() -> void
{
    const auto tokenType = m_previous->tokenType();
    const auto runtimeType = static_cast<runtime::types::Type>(tokenType);

    if (match(scanner::TokenType::OpenParen)) {
        // constructing an instance of the type
        if (const auto args = parseCallArgs(scanner::TokenType::CloseParen)) {
            const auto [numArgs, hasUnpack] = *args;

            switch (scanner::builtinConstructorAllowedArgCount(tokenType)) {
                case scanner::AllowedArgCount::None: {
                    if (numArgs != 0_uz) {
                        errorAtPrevious(fmt::format("{} takes no arguments", runtimeType));
                        return;
                    }

                    break;
                }
                case scanner::AllowedArgCount::NoneOrOneOrTwo: {
                    if (numArgs > 2_uz) {
                        errorAtPrevious(fmt::format("{} takes <= 2 arguments", runtimeType));
                        return;
                    }

                    break;
                }
                case scanner::AllowedArgCount::One: {
                    if (numArgs != 1_uz) {
                        errorAtPrevious(fmt::format("{} takes one argument", runtimeType));
                        return;
                    }

                    break;
                }
                case scanner::AllowedArgCount::OneOrNone: {
                    if (numArgs > 1_uz) {
                        errorAtPrevious(fmt::format("{} takes <= 1 arguments", runtimeType));
                        return;
                    }

                    break;
                }
                case scanner::AllowedArgCount::OneOrMore: {
                    if (numArgs < 1_uz) {
                        errorAtPrevious(fmt::format("{} takes >= 1 arguments", runtimeType));
                        return;
                    }

                    break;
                }
                case scanner::AllowedArgCount::TwoOrThree: {
                    if (numArgs < 2_uz || numArgs > 3_u8) {
                        errorAtPrevious(fmt::format("{} takes two or three arguments", runtimeType));
                        return;
                    }

                    break;
                }
                case scanner::AllowedArgCount::Any:
                    break;
            }

            emitConstant(static_cast<u8>(tokenType));
            emitConstant(numArgs);
            emitConstant(hasUnpack);
            if (tokenType == scanner::TokenType::RangeIdent) {
                emitConstant(false);
            }
            emitOp(runtime::Op::ConstructBuiltin, m_previous->line());
        }
    } else {
        // just loading the type itself
        emitConstant(static_cast<u8>(tokenType));
        emitOp(runtime::Op::LoadType, m_previous->line());
    }
}

auto Compiler::typeOf() -> void
{
    RETURN_IF_NO_MATCH(scanner::TokenType::OpenParen, "Expected '('");
    expression(false, false);
    emitOp(runtime::Op::TypeOf, m_previous->line());
    RETURN_IF_NO_MATCH(scanner::TokenType::CloseParen, "Expected ')");
}

auto Compiler::lambda() -> void
{
    std::vector<usize> captureIndexes;
    std::vector<LocalVariable> captures;

    while (!match(scanner::TokenType::Pipe)) {
        if (match(scanner::TokenType::Identifier)) {
            if (captures.size() == std::numeric_limits<u8>::max()) {
                errorAtCurrent("Maximum amount of captures exceeded");
                return;
            }

            const auto text = m_previous->text();
            if (const auto localIndex = indexOfLocal(text)) {
                if (std::ranges::find_if(captures, [&text] (const LocalVariable& local) -> bool {
                    return local.name == text;
                }) != captures.end()) {
                    errorAtPrevious(fmt::format("Local variable '{}' has already been captured", text));
                    return;
                }

                captures.push_back(m_localNames[*localIndex]);
                captureIndexes.push_back(*localIndex);

                // trailing commas are allowed but all arguments must be comma separated
                // so here, if the next token is not a comma or a pipe, it's invalid
                if (!check(scanner::TokenType::Pipe) && !check(scanner::TokenType::Comma)) {
                    errorAtCurrent("Expected ',' or '|'");
                    break;
                }

                if (check(scanner::TokenType::Comma)) {
                    advance();
                }
            } else {
                errorAtPrevious(fmt::format("No local variable named '{}' to capture", text));
            }
        } else {
            errorAtCurrent("Expected identifier for capture");
        }
    }

    auto oldLocals = std::move(m_localNames);
    m_localNames = std::move(captures);

    std::optional<FunctionParamsParseResult> params;
    if (match(scanner::TokenType::OpenParen)) {
        params = parseFunctionParams(true);
        if (!params) {
            return;
        }
    } else {
        params = FunctionParamsParseResult{.numParams = 0, .hasVariadicParams = false, .extensionFunctionTypes = {}};
    }

    const auto [arity, hasVariadicParams, extensionFunctionType] = *params;

    if (match(scanner::TokenType::Colon)) {
        parseTypeAnnotation();
    }

    m_contextStack.push_back(Context::Lambda);

    const auto prevFunction = m_vm->currentFunction();
    auto lambdaName = fmt::format("{}_lambda{}", prevFunction->name(), prevFunction->numLambdas());

    // untracked because this lives in the constant list
    // during runtime, a shallow clone is made which IS tracked along with its captures
    auto lambda = runtime::Value::createObjectUntracked<objects::Function>(std::move(lambdaName), m_filePath, m_filePathHash, arity, false, hasVariadicParams);
    auto functionPtr = lambda.object()->asFunction();
    m_vm->setCurrentFunction(functionPtr);

    for (auto i = 0_uz; i < m_localNames.size() - arity; i++) {
        emitConstant(i);
        emitOp(runtime::Op::LoadCapture, m_previous->line());
    }

    if (match(scanner::TokenType::OpenBrace)) {
        if (!parseBlock("lambda")) {
            return;
        }
    } else if (match(scanner::TokenType::Arrow)) {
        // could be an expression, or a statement
        // if it's a statement that's NOT a return, just parse that and return none
        // if it's a return statement, nothing to be done
        if (scanner::isValidStartOfExpression(m_current->tokenType())){
            expression(true, false);
            emitConstant(0);
            if (lastOpWasAssignment()) {
                emitConstant(runtime::Value::none());
                emitOp(runtime::Op::LoadConstant, m_previous->line());
            }
            emitOp(runtime::Op::PopLocals, m_previous->line());
            emitOp(runtime::Op::Return, m_previous->line());
        } else {
            statement(false);
        }
    } else {
        m_vm->setCurrentFunction(prevFunction);
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

#ifdef POISE_DEBUG
    functionPtr->printOps();
#endif

    m_localNames = std::move(oldLocals);
    m_contextStack.pop_back();

    m_vm->setCurrentFunction(prevFunction);
    prevFunction->lamdaAdded();

    emitConstant(std::move(lambda));
    emitOp(runtime::Op::MakeLambda, m_previous->line());
    for (const auto index : captureIndexes) {
        emitConstant(index);
        emitOp(runtime::Op::CaptureLocal, m_previous->line());
    }
}

auto Compiler::list() -> void
{
    const auto args = parseCallArgs(scanner::TokenType::CloseSquareBracket);
    if (!args) {
        return;
    }

    const auto [numArgs, hasUnpack] = *args;
    emitConstant(static_cast<u8>(runtime::types::Type::List));
    emitConstant(numArgs);
    emitConstant(hasUnpack);
    emitOp(runtime::Op::ConstructBuiltin, m_previous->line());
}

auto Compiler::tupleOrGrouping() -> void
{
    if (match(scanner::TokenType::DotDotDot)) {
        unpack();
        emitConstant(static_cast<u8>(runtime::types::Type::Tuple));
        emitConstant(1);
        emitConstant(true);
        emitOp(runtime::Op::ConstructBuiltin, m_previous->line());
        RETURN_IF_NO_MATCH(scanner::TokenType::CloseParen, "Expected ')'");
    } else {
        expression(false, false);

        if (match(scanner::TokenType::Comma)) {
            if (const auto args = parseCallArgs(scanner::TokenType::CloseParen)) {
                const auto [numArgs, hasUnpack] = *args;
                emitConstant(static_cast<u8>(runtime::types::Type::Tuple));
                emitConstant(numArgs + 1);
                emitConstant(hasUnpack);
                emitOp(runtime::Op::ConstructBuiltin, m_previous->line());
            }
        } else {
            RETURN_IF_NO_MATCH(scanner::TokenType::CloseParen, "Expected ')'");
        }
    }
    
}

auto Compiler::dict() -> void
{
    const auto args = parseCallArgs(scanner::TokenType::CloseBrace);
    if (!args) {
        return;
    }

    const auto [numArgs, hasUnpack] = *args;
    emitConstant(static_cast<u8>(runtime::types::Type::Dict));
    emitConstant(numArgs);
    emitConstant(hasUnpack);
    emitOp(runtime::Op::ConstructBuiltin, m_previous->line());
}

static auto getEscapeCharacter(char c) -> std::optional<char>
{
    switch (c) {
        case 't':
            return '\t';
        case 'n':
            return '\n';
        case 'r':
            return '\r';
        case '"':
            return '"';
        case '\\':
            return '\\';
        default:
            return {};
    }
}

auto Compiler::parseString() -> std::optional<std::string>
{
    std::string result;
    const auto tokenText = m_previous->text();
    auto i = 1_uz;
    while (i < tokenText.length() - 1_uz) {
        if (tokenText[i] == '\\') {
            i++;

            if (i == tokenText.length() - 1_uz) {
                errorAtPrevious("Expected escape character but string terminated");
                return {};
            }

            if (const auto escapeChar = getEscapeCharacter(tokenText[i])) {
                result.push_back(*escapeChar);
            } else {
                errorAtPrevious(fmt::format("Unrecognised escape character '{}'", tokenText[i]));
                return {};
            }
        } else {
            result.push_back(tokenText[i]);
        }

        i++;
    }

    return {std::move(result)};
}

auto Compiler::parseInt() -> std::optional<i64>
{
    auto value{0_i64};
    const auto text = m_previous->text();

    const auto isBinary = text.size() >= 2_uz && (text[1_uz] == 'b' || text[1_uz] == 'B');
    const auto isHex = text.size() >= 2_uz && (text[1_uz] == 'x' || text[1_uz] == 'X');
    
    std::from_chars_result result{};
    if (isBinary) {
        if (text[0_uz] != '0') {
            errorAtPrevious("Binary literals must start with '0'");
            return {};
        }

        std::string cleaned;
        for (auto i = 2_uz; i < text.size(); i++) {
            if (text[i] == '_') {
                continue;
            }

            if (text[i] != '0' && text[i] != '1' && text[i] != '_') {
                errorAtPrevious("Binary literals must only contain '1' and '0'");
                return {};
            }

            cleaned.push_back(text[i]);
        }

        result = std::from_chars(cleaned.data(), cleaned.data() + cleaned.size(), value, 2);
    } else if (isHex) {
        if (text[0_uz] != '0') {
            errorAtPrevious("Hex literals must start with '0'");
            return {};
        }

        auto isHexChar = [] (char c) -> bool {
            return std::isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
        };

        std::string cleaned;
        for (auto i = 2_uz; i < text.size(); i++) {
            if (text[i] == '_') {
                continue;
            }

            if (!isHexChar(text[i]) && text[i] != '_') {
                errorAtPrevious("Hex literals must only contain digits or characters in the range 'A' to 'F'");
                return {};
            }

            cleaned.push_back(text[i]);
        }

        result = std::from_chars(cleaned.data(), cleaned.data() + cleaned.size(), value, 16);
    } else {
        std::string cleaned;
        for (auto i = 0_uz; i < text.size(); i++) {
            if (text[i] != '_') {
                cleaned.push_back(text[i]);
            }
        }

        result = std::from_chars(cleaned.data(), cleaned.data() + cleaned.size(), value);
    }

    const auto [ptr, ec] = result;

    if (ec == std::errc{}) {
        return value;
    } else if (ec == std::errc::invalid_argument) {
        errorAtPrevious(fmt::format("Unable to parse Int '{}', failed at '{}'", text, *ptr));
    } else if (ec == std::errc::result_out_of_range) {
        errorAtPrevious(fmt::format("Int out of range '{}'", text));
    }

    return {};
}

auto Compiler::parseFloat() -> std::optional<f64>
{
#ifdef _LIBCPP_VERSION
    try {
        const auto text = m_previous->string();
        const auto result = std::stod(text);
        return result;
    } catch (const std::invalid_argument&) {
        errorAtPrevious(fmt::format("Invalid Float '{}'", m_previous->text()));
    } catch (const std::out_of_range&) {
        errorAtPrevious(fmt::format("Float '{}' out of range", m_previous->text()));
    }

    return {};
#else
    auto result{0.0};
    const auto text = m_previous->string();
    const auto [_, ec] = std::from_chars(text.data(), text.data() + text.length(), result);

    if (ec == std::errc{}) {
        return result;
    } else if (ec == std::errc::invalid_argument) {
        errorAtPrevious(fmt::format("Unable to parse Float '{}'", text));
    } else if (ec == std::errc::result_out_of_range) {
        errorAtPrevious(fmt::format("Float out of range '{}'", text));
    }

    return {};
#endif
}
}   // namespace poise::compiler
