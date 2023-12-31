//
// Created by Ryan Jeffares on 15/12/2023.
//

#include "Compiler.hpp"
#include "Compiler_Macros.hpp"

#include <charconv>

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
        range(canAssign, canUnpack);
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
    // TODO: unpack calls
    RETURN_IF_NO_MATCH(scanner::TokenType::Identifier, "Expected identifier");

    const auto text = m_previous->text();
    if (const auto index = indexOfLocal(text)) {
        emitConstant(*index);
        emitOp(runtime::Op::LoadLocal, m_previous->line());
        emitOp(runtime::Op::Unpack, m_previous->line());
    }
}

auto Compiler::range(bool canAssign, bool canUnpack) -> void
{
    logicOr(canAssign, canUnpack);

    if (match(scanner::TokenType::DotDot) || match(scanner::TokenType::DotDotEqual)) {
        const auto inclusive = m_previous->tokenType() == scanner::TokenType::DotDotEqual;
        logicOr(canAssign, canUnpack);

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

auto Compiler::logicOr(bool canAssign, bool canUnpack) -> void
{
    logicAnd(canAssign, canUnpack);

    std::optional<JumpIndexes> jumpIndexes;
    if (check(scanner::TokenType::Or)) {
        jumpIndexes = emitJump(JumpType::IfTrue, false);
    }

    while (match(scanner::TokenType::Or)) {
        logicAnd(canAssign, canUnpack);
        emitOp(runtime::Op::LogicOr, m_previous->line());
    }

    if (jumpIndexes) {
        patchJump(*jumpIndexes);
    }
}

auto Compiler::logicAnd(bool canAssign, bool canUnpack) -> void
{
    bitwiseOr(canAssign, canUnpack);

    std::optional<JumpIndexes> jumpIndexes;
    if (check(scanner::TokenType::And)) {
        jumpIndexes = emitJump(JumpType::IfFalse, false);
    }

    while (match(scanner::TokenType::And)) {
        bitwiseOr(canAssign, canUnpack);
        emitOp(runtime::Op::LogicAnd, m_previous->line());
    }

    if (jumpIndexes) {
        patchJump(*jumpIndexes);
    }
}

auto Compiler::bitwiseOr(bool canAssign, bool canUnpack) -> void
{
    bitwiseXor(canAssign, canUnpack);

    while (match(scanner::TokenType::Pipe)) {
        bitwiseXor(canAssign, canUnpack);
        emitOp(runtime::Op::BitwiseOr, m_previous->line());
    }
}

auto Compiler::bitwiseXor(bool canAssign, bool canUnpack) -> void
{
    bitwiseAnd(canAssign, canUnpack);

    while (match(scanner::TokenType::Caret)) {
        bitwiseAnd(canAssign, canUnpack);
        emitOp(runtime::Op::BitwiseXor, m_previous->line());
    }
}

auto Compiler::bitwiseAnd(bool canAssign, bool canUnpack) -> void
{
    equality(canAssign, canUnpack);

    while (match(scanner::TokenType::Ampersand)) {
        equality(canAssign, canUnpack);
        emitOp(runtime::Op::BitwiseAnd, m_previous->line());
    }
}

auto Compiler::equality(bool canAssign, bool canUnpack) -> void
{
    comparison(canAssign, canUnpack);

    if (match(scanner::TokenType::EqualEqual)) {
        comparison(canAssign, canUnpack);
        emitOp(runtime::Op::Equal, m_previous->line());
    } else if (match(scanner::TokenType::NotEqual)) {
        comparison(canAssign, canUnpack);
        emitOp(runtime::Op::NotEqual, m_previous->line());
    }
}

auto Compiler::comparison(bool canAssign, bool canUnpack) -> void
{
    shift(canAssign, canUnpack);

    if (match(scanner::TokenType::Less)) {
        shift(canAssign, canUnpack);
        emitOp(runtime::Op::LessThan, m_previous->line());
    } else if (match(scanner::TokenType::LessEqual)) {
        shift(canAssign, canUnpack);
        emitOp(runtime::Op::LessEqual, m_previous->line());
    } else if (match(scanner::TokenType::Greater)) {
        shift(canAssign, canUnpack);
        emitOp(runtime::Op::GreaterThan, m_previous->line());
    } else if (match(scanner::TokenType::GreaterEqual)) {
        shift(canAssign, canUnpack);
        emitOp(runtime::Op::GreaterEqual, m_previous->line());
    }
}

auto Compiler::shift(bool canAssign, bool canUnpack) -> void
{
    term(canAssign, canUnpack);

    while (true) {
        if (match(scanner::TokenType::ShiftLeft)) {
            term(canAssign, canUnpack);
            emitOp(runtime::Op::LeftShift, m_previous->line());
        } else if (match(scanner::TokenType::ShiftRight)) {
            term(canAssign, canUnpack);
            emitOp(runtime::Op::RightShift, m_previous->line());
        } else {
            break;
        }
    }
}

auto Compiler::term(bool canAssign, bool canUnpack) -> void
{
    factor(canAssign, canUnpack);

    while (true) {
        if (match(scanner::TokenType::Plus)) {
            factor(canAssign, canUnpack);
            emitOp(runtime::Op::Addition, m_previous->line());
        } else if (match(scanner::TokenType::Minus)) {
            factor(canAssign, canUnpack);
            emitOp(runtime::Op::Subtraction, m_previous->line());
        } else {
            break;
        }
    }
}

auto Compiler::factor(bool canAssign, bool canUnpack) -> void
{
    unary(canAssign, canUnpack);

    while (true) {
        if (match(scanner::TokenType::Star)) {
            unary(canAssign, canUnpack);
            emitOp(runtime::Op::Multiply, m_previous->line());
        } else if (match(scanner::TokenType::Slash)) {
            unary(canAssign, canUnpack);
            emitOp(runtime::Op::Divide, m_previous->line());
        } else if (match(scanner::TokenType::Modulus)) {
            unary(canAssign, canUnpack);
            emitOp(runtime::Op::Modulus, m_previous->line());
        } else {
            break;
        }
    }
}

auto Compiler::unary(bool canAssign, bool canUnpack) -> void
{
    if (match(scanner::TokenType::Minus)) {
        const auto line = m_previous->line();
        unary(canAssign, canUnpack);
        emitOp(runtime::Op::Negate, line);
    } else if (match(scanner::TokenType::Tilde)) {
        const auto line = m_previous->line();
        unary(canAssign, canUnpack);
        emitOp(runtime::Op::BitwiseNot, line);
    } else if (match(scanner::TokenType::Exclamation)) {
        const auto line = m_previous->line();
        unary(canAssign, canUnpack);
        emitOp(runtime::Op::LogicNot, line);
    } else if (match(scanner::TokenType::Plus)) {
        auto line = m_previous->line();
        unary(canAssign, canUnpack);
        emitOp(runtime::Op::Plus, line);
    } else {
        call(canAssign, canUnpack);
    }
}

auto Compiler::call(bool canAssign, bool canUnpack) -> void
{
    primary(canAssign, canUnpack);

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
            emitConstant(m_previous->string());
            emitConstant(m_stringHasher(m_previous->string()));
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
            }
        } else {
            break;
        }
    }
}

auto Compiler::primary(bool canAssign, bool canUnpack) -> void
{
    if (match(scanner::TokenType::False)) {
        emitConstant(false);
        emitOp(runtime::Op::LoadConstant, m_previous->line());
    } else if (match(scanner::TokenType::True)) {
        emitConstant(true);
        emitOp(runtime::Op::LoadConstant, m_previous->line());
    } else if (match(scanner::TokenType::Float)) {
        parseFloat();
    } else if (match(scanner::TokenType::Int)) {
        parseInt();
    } else if (match(scanner::TokenType::None)) {
        emitConstant(runtime::Value::none());
        emitOp(runtime::Op::LoadConstant, m_previous->line());
    } else if (match(scanner::TokenType::String)) {
        parseString();
    } else if (match(scanner::TokenType::OpenParen)) {
        expression(false, canUnpack);
        RETURN_IF_NO_MATCH(scanner::TokenType::CloseParen, "Expected ')'");
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
            emitConstant(m_stringHasher(identifier));
            emitConstant(std::move(identifier));
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
        return;
    }
}

auto Compiler::namespaceQualifiedCall() -> void
{
    // currently, the only things you can have at the top level that you could try and load
    // are functions, and since we've already compiled the file you're looking in, we can verify this now
    // TODO: constants

    // so first, parse the rest of the namespace...
    const auto identifier = m_previous->string();

    std::filesystem::path namespaceFilePath;
    std::string namespaceText = identifier;

    advance(); // consume the first '::'

    if (m_importAliasLookup.contains(namespaceText)) {
        namespaceFilePath = m_importAliasLookup[namespaceText];
        advance(); // consume the function name
    } else {
        if (identifier == "std") {
            if (auto stdPath = getStdPath()) {
                namespaceFilePath.swap(*stdPath);
            } else {
                errorAtPrevious("The environment variable `POISE_STD_PATH` has not been set, cannot open std file");
                return;
            }
        } else {
            namespaceFilePath = m_filePath.parent_path() / identifier;
        }

        // current token is EITHER the next part of the namespace OR the function
        while (match(scanner::TokenType::Identifier)) {
            auto text = m_previous->text();

            if (match(scanner::TokenType::ColonColon)) {
                // continue namespace
                namespaceFilePath /= text;
                namespaceText += "::";
                namespaceText += text;
            } else {
                break;
            }
        }
    }

    auto verifyNamespaceAndFunction =
        [this](std::string_view functionName, std::string_view namespaceText, usize namespaceHash) -> bool {
            const auto namespaceManager = m_vm->namespaceManager();
            // load the function
            if (!namespaceManager->namespaceHasImportedNamespace(m_filePathHash, namespaceHash)) {
                errorAtPrevious(fmt::format("Namespace '{}' not imported", namespaceText));
                return false;
            }

            if (auto function = namespaceManager->namespaceFunction(namespaceHash, functionName)) {
                if (function->exported()) {
                    return true;
                } else {
                    errorAtPrevious(fmt::format("Function '{}' is not exported", functionName));
                    return false;
                }
            } else {
                errorAtPrevious(fmt::format("Function '{}' not found in namespace '{}'", functionName, namespaceText));
                return false;
            }
        };

    auto functionName = m_previous->string();
    const auto namespaceManager = m_vm->namespaceManager();

    if (match(scanner::TokenType::OpenParen)) {
        // function call
        // consume semicolon
        if (!namespaceFilePath.has_extension()) {
            namespaceFilePath += ".poise";
        }

        const auto namespaceHash = namespaceManager->namespaceHash(namespaceFilePath);
        if (!verifyNamespaceAndFunction(functionName, namespaceText, namespaceHash)) {
            return;
        }

        emitConstant(namespaceHash);
        emitConstant(m_stringHasher(functionName));
        emitConstant(functionName);
        emitOp(runtime::Op::LoadFunction, m_previous->line());

        if (const auto args = parseCallArgs(scanner::TokenType::CloseParen)) {
            const auto function = namespaceManager->namespaceFunction(namespaceHash, functionName);
            const auto [numArgs, hasUnpack] = *args;

            if (hasUnpack || function->hasVariadicParams()) {
                if (numArgs < function->arity()) {
                    errorAtPrevious(fmt::format("Expected >={} args to '{}::{}()' but got {}", function->arity(), namespaceText, functionName, numArgs));
                    return;
                }
            } else {
                if (numArgs != function->arity()) {
                    errorAtPrevious(fmt::format("Expected {} args to '{}::{}()' but got {}", function->arity(), namespaceText, functionName, numArgs));
                    return;
                }
            }

            emitConstant(numArgs);
            emitConstant(hasUnpack);
            emitConstant(false);
            emitOp(runtime::Op::Call, m_previous->line());
        }
    } else {
        if (!namespaceFilePath.has_extension()) {
            namespaceFilePath += ".poise";
        }

        const auto namespaceHash = namespaceManager->namespaceHash(namespaceFilePath);
        if (!verifyNamespaceAndFunction(functionName, namespaceText, namespaceHash)) {
            return;
        }

        emitConstant(namespaceHash);
        emitConstant(m_stringHasher(functionName));
        emitConstant(std::move(functionName));
        emitOp(runtime::Op::LoadFunction, m_previous->line());
    }
}

auto Compiler::typeIdent() -> void
{
    const auto tokenType = m_previous->tokenType();

    if (match(scanner::TokenType::OpenParen)) {
        // constructing an instance of the type
        if (const auto args = parseCallArgs(scanner::TokenType::CloseParen)) {
            const auto [numArgs, hasUnpack] = *args;
            if (tokenType < scanner::TokenType::ListIdent) {
                if (numArgs > 1) {
                    errorAtPrevious(fmt::format("'{}' can only be constructed from a single argument but was given {}", static_cast<runtime::types::Type>(tokenType), numArgs));
                    return;
                }
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
                if (std::find_if(captures.cbegin(), captures.cend(), [&text](const LocalVariable& local) {
                    return local.name == text;
                }) != captures.cend()) {
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

    m_contextStack.push_back(Context::Function);

    const auto prevFunction = m_vm->currentFunction();
    auto lambdaName = fmt::format("{}_lambda{}", prevFunction->name(), prevFunction->numLambdas());
    auto lambda = runtime::Value::createObject<objects::PoiseFunction>(std::move(lambdaName), m_filePath, m_filePathHash, arity, false, hasVariadicParams);
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
    emitConstant(numArgs);
    emitConstant(hasUnpack);
    emitOp(runtime::Op::MakeList, m_previous->line());
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

auto Compiler::parseString() -> void
{
    std::string result;
    const auto tokenText = m_previous->text();
    auto i = 1_uz;
    while (i < tokenText.length() - 1_uz) {
        if (tokenText[i] == '\\') {
            i++;

            if (i == tokenText.length() - 1_uz) {
                errorAtPrevious("Expected escape character but string terminated");
                return;
            }

            if (const auto escapeChar = getEscapeCharacter(tokenText[i])) {
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
    auto result{0_i64};
    const auto text = m_previous->text();
    const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.length(), result);

    if (ec == std::errc{}) {
        emitConstant(result);
        emitOp(runtime::Op::LoadConstant, m_previous->line());
    } else if (ec == std::errc::invalid_argument) {
        errorAtPrevious(fmt::format("Unable to parse Int '{}'", text));
        return;
    } else if (ec == std::errc::result_out_of_range) {
        errorAtPrevious(fmt::format("Int out of range '{}'", text));
        return;
    }
}

auto Compiler::parseFloat() -> void
{
    auto result{0.0};
    const auto text = m_previous->string();
    const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.length(), result);

    if (ec == std::errc{}) {
        emitConstant(result);
        emitOp(runtime::Op::LoadConstant, m_previous->line());
    } else if (ec == std::errc::invalid_argument) {
        errorAtPrevious(fmt::format("Unable to parse Float '{}'", text));
        return;
    } else if (ec == std::errc::result_out_of_range) {
        errorAtPrevious(fmt::format("Float out of range '{}'", text));
        return;
    }
}
}   // namespace poise::compiler
