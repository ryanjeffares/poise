#include "TokenType.hpp"

namespace poise::scanner {
auto isLiteral(TokenType tokenType) noexcept -> bool
{
    switch (tokenType) {
        case TokenType::False:
        case TokenType::Float:
        case TokenType::Identifier:
        case TokenType::Int:
        case TokenType::None:
        case TokenType::String:
        case TokenType::True:
            return true;
        default:
            return false;
    }
}

auto isUnaryOp(TokenType tokenType) noexcept -> bool
{
    switch (tokenType) {
        case TokenType::Exclamation:
        case TokenType::Tilde:
        case TokenType::Minus:
        case TokenType::Plus:
            return true;
        default:
            return false;
    }
}

auto isTypeIdent(TokenType tokenType) noexcept -> bool
{
    switch (tokenType) {   
        case TokenType::BoolIdent:
        case TokenType::FloatIdent:
        case TokenType::IntIdent:
        case TokenType::NoneIdent:
        case TokenType::StringIdent:
        case TokenType::DictIdent:
        case TokenType::ExceptionIdent:
        case TokenType::FunctionIdent:
        case TokenType::ListIdent:
        case TokenType::RangeIdent:
        case TokenType::SetIdent:
        case TokenType::TupleIdent:
            return true;
        default:
            return false;
    }
}

auto isPrimitiveTypeIdent(TokenType tokenType) noexcept -> bool
{
    switch (tokenType) {
        case TokenType::BoolIdent:
        case TokenType::FloatIdent:
        case TokenType::IntIdent:
        case TokenType::NoneIdent:
        case TokenType::StringIdent:
            return true;
        default:
            return false;
    }
}

auto isBuiltinFunction(TokenType tokenType) noexcept -> bool
{
    switch (tokenType) {
        case TokenType::TypeOf:
            return true;
        default:
            return false;
    }
}

auto isValidStartOfExpression(TokenType tokenType) noexcept -> bool
{
    return isLiteral(tokenType) ||
           isUnaryOp(tokenType) ||
           isTypeIdent(tokenType) ||
           isBuiltinFunction(tokenType) ||
           tokenType == TokenType::OpenParen ||         // for groupings
           tokenType == TokenType::Pipe ||              // for lambdas
           tokenType == TokenType::OpenSquareBracket || // for lists
           tokenType == TokenType::OpenBrace;           // for dicts 
}

auto typeDisplayName(TokenType tokenType) noexcept -> std::string_view
{
    POISE_ASSERT(isTypeIdent(tokenType), fmt::format("Expected type ident that can be generic but got {}", tokenType));

    switch (tokenType) {
        case TokenType::BoolIdent:
            return "Bool";
        case TokenType::FloatIdent:
            return "Float";
        case TokenType::IntIdent:
            return "Int";
        case TokenType::NoneIdent:
            return "None";
        case TokenType::StringIdent:
            return "String";
        case TokenType::DictIdent:
            return "Dict";
        case TokenType::ExceptionIdent:
            return "Exception";
        case TokenType::FunctionIdent:
            return "Function";
        case TokenType::ListIdent:
            return "List";
        case TokenType::RangeIdent:
            return "Range";
        case TokenType::SetIdent:
            return "Set";
        case TokenType::TupleIdent:
            return "Tuple";
        default:
            POISE_UNREACHABLE();
    }
}

auto builtinGenericTypeCount(TokenType tokenType) noexcept -> AllowedGenericTypeCount
{
    switch (tokenType) {
        case TokenType::ListIdent:
        case TokenType::SetIdent:
            return AllowedGenericTypeCount::One;
        case TokenType::DictIdent:
            return AllowedGenericTypeCount::Two;
        case TokenType::TupleIdent:
            return AllowedGenericTypeCount::Any;
        default:
            return AllowedGenericTypeCount::None;
    }
}

auto builtinConstructorAllowedArgCount(TokenType tokenType) noexcept -> AllowedArgCount
{
    POISE_ASSERT(isTypeIdent(tokenType), fmt::format("Expected type ident but got {}", tokenType));

    switch (tokenType) {   
        case TokenType::NoneIdent:
            return AllowedArgCount::None;
        case TokenType::ExceptionIdent:
            return AllowedArgCount::NoneOrOneOrTwo;
        case TokenType::FunctionIdent:
            return AllowedArgCount::One;
        case TokenType::BoolIdent:
        case TokenType::FloatIdent:
        case TokenType::IntIdent:
        case TokenType::StringIdent:
            return AllowedArgCount::OneOrNone;
        case TokenType::TupleIdent:
            return AllowedArgCount::OneOrMore;
        case TokenType::RangeIdent:
            return AllowedArgCount::TwoOrThree;
        case TokenType::DictIdent:
        case TokenType::ListIdent:
        case TokenType::SetIdent:
            return AllowedArgCount::Any;
        default:
            return AllowedArgCount::None;
    }
}
}   // namespace poise::scanner

namespace fmt {
using namespace poise;

auto formatter<scanner::TokenType>::format(scanner::TokenType tokenType, format_context& context) const -> decltype(context.out())
{
    switch (tokenType) {
        case scanner::TokenType::BoolIdent:
            return formatter<string_view>::format("BoolIdent", context);
        case scanner::TokenType::FloatIdent:
            return formatter<string_view>::format("FloatIdent", context);
        case scanner::TokenType::IntIdent:
            return formatter<string_view>::format("IntIdent", context);
        case scanner::TokenType::NoneIdent:
            return formatter<string_view>::format("NoneIdent", context);
        case scanner::TokenType::StringIdent:
            return formatter<string_view>::format("StringIdent", context);
        case scanner::TokenType::DictIdent:
            return formatter<string_view>::format("DictIdent", context);
        case scanner::TokenType::ExceptionIdent:
            return formatter<string_view>::format("ExceptionIdent", context);
        case scanner::TokenType::FunctionIdent:
            return formatter<string_view>::format("FunctionIdent", context);
        case scanner::TokenType::ListIdent:
            return formatter<string_view>::format("ListIdent", context);
        case scanner::TokenType::RangeIdent:
            return formatter<string_view>::format("RangeIdent", context);
        case scanner::TokenType::SetIdent:
            return formatter<string_view>::format("SetIdent", context);
        case scanner::TokenType::TupleIdent:
            return formatter<string_view>::format("TupleIdent", context);
        case scanner::TokenType::And:
            return formatter<string_view>::format("And", context);
        case scanner::TokenType::As:
            return formatter<string_view>::format("As", context);
        case scanner::TokenType::Break:
            return formatter<string_view>::format("Break", context);
        case scanner::TokenType::By:
            return formatter<string_view>::format("By", context);
        case scanner::TokenType::Catch:
            return formatter<string_view>::format("Catch", context);
        case scanner::TokenType::Const:
            return formatter<string_view>::format("Const", context);
        case scanner::TokenType::Continue:
            return formatter<string_view>::format("Continue", context);
        case scanner::TokenType::Else:
            return formatter<string_view>::format("Else", context);
        case scanner::TokenType::EPrint:
            return formatter<string_view>::format("EPrint", context);
        case scanner::TokenType::EPrintLn:
            return formatter<string_view>::format("EPrintLn", context);
        case scanner::TokenType::Export:
            return formatter<string_view>::format("Export", context);
        case scanner::TokenType::Final:
            return formatter<string_view>::format("Final", context);
        case scanner::TokenType::For:
            return formatter<string_view>::format("For", context);
        case scanner::TokenType::Func:
            return formatter<string_view>::format("Func", context);
        case scanner::TokenType::If:
            return formatter<string_view>::format("If", context);
        case scanner::TokenType::Import:
            return formatter<string_view>::format("Import", context);
        case scanner::TokenType::In:
            return formatter<string_view>::format("In", context);
        case scanner::TokenType::Or:
            return formatter<string_view>::format("Or", context);
        case scanner::TokenType::Print:
            return formatter<string_view>::format("Print", context);
        case scanner::TokenType::PrintLn:
            return formatter<string_view>::format("PrintLn", context);
        case scanner::TokenType::Return:
            return formatter<string_view>::format("Return", context);
        case scanner::TokenType::Struct:
            return formatter<string_view>::format("Struct", context);
        case scanner::TokenType::Throw:
            return formatter<string_view>::format("Throw", context);
        case scanner::TokenType::This:
            return formatter<string_view>::format("This", context);
        case scanner::TokenType::Try:
            return formatter<string_view>::format("Try", context);
        case scanner::TokenType::TypeOf:
            return formatter<string_view>::format("TypeOf", context);
        case scanner::TokenType::Var:
            return formatter<string_view>::format("Var", context);
        case scanner::TokenType::While:
            return formatter<string_view>::format("While", context);
        case scanner::TokenType::Ampersand:
            return formatter<string_view>::format("Ampersand", context);
        case scanner::TokenType::Arrow:
            return formatter<string_view>::format("Arrow", context);
        case scanner::TokenType::Caret:
            return formatter<string_view>::format("Caret", context);
        case scanner::TokenType::CloseBrace:
            return formatter<string_view>::format("CloseBrace", context);
        case scanner::TokenType::CloseParen:
            return formatter<string_view>::format("CloseParen", context);
        case scanner::TokenType::CloseSquareBracket:
            return formatter<string_view>::format("CloseSquareBracket", context);
        case scanner::TokenType::Colon:
            return formatter<string_view>::format("Colon", context);
        case scanner::TokenType::ColonColon:
            return formatter<string_view>::format("ColonColon", context);
        case scanner::TokenType::Comma:
            return formatter<string_view>::format("Comma", context);
        case scanner::TokenType::Dot:
            return formatter<string_view>::format("Dot", context);
        case scanner::TokenType::DotDot:
            return formatter<string_view>::format("DotDot", context);
        case scanner::TokenType::DotDotDot:
            return formatter<string_view>::format("DotDotDot", context);
        case scanner::TokenType::DotDotEqual:
            return formatter<string_view>::format("DotDotEqual", context);
        case scanner::TokenType::Equal:
            return formatter<string_view>::format("Equal", context);
        case scanner::TokenType::EqualEqual:
            return formatter<string_view>::format("EqualEqual", context);
        case scanner::TokenType::Exclamation:
            return formatter<string_view>::format("Exclamation", context);
        case scanner::TokenType::Greater:
            return formatter<string_view>::format("Greater", context);
        case scanner::TokenType::GreaterEqual:
            return formatter<string_view>::format("GreaterEqual", context);
        case scanner::TokenType::Less:
            return formatter<string_view>::format("Less", context);
        case scanner::TokenType::LessEqual:
            return formatter<string_view>::format("LessEqual", context);
        case scanner::TokenType::Modulus:
            return formatter<string_view>::format("Modulus", context);
        case scanner::TokenType::Minus:
            return formatter<string_view>::format("Subtraction", context);
        case scanner::TokenType::NotEqual:
            return formatter<string_view>::format("NotEqual", context);
        case scanner::TokenType::OpenBrace:
            return formatter<string_view>::format("OpenBrace", context);
        case scanner::TokenType::OpenParen:
            return formatter<string_view>::format("OpenParen", context);
        case scanner::TokenType::OpenSquareBracket:
            return formatter<string_view>::format("OpenSquareBracket", context);
        case scanner::TokenType::Pipe:
            return formatter<string_view>::format("Pipe", context);
        case scanner::TokenType::Plus:
            return formatter<string_view>::format("Addition", context);
        case scanner::TokenType::Semicolon:
            return formatter<string_view>::format("Semicolon", context);
        case scanner::TokenType::ShiftLeft:
            return formatter<string_view>::format("ShiftLeft", context);
        case scanner::TokenType::ShiftRight:
            return formatter<string_view>::format("ShiftRight", context);
        case scanner::TokenType::Slash:
            return formatter<string_view>::format("Slash", context);
        case scanner::TokenType::Star:
            return formatter<string_view>::format("Star", context);
        case scanner::TokenType::Tilde:
            return formatter<string_view>::format("Tilde", context);
        case scanner::TokenType::False:
            return formatter<string_view>::format("False", context);
        case scanner::TokenType::Float:
            return formatter<string_view>::format("Float", context);
        case scanner::TokenType::Identifier:
            return formatter<string_view>::format("Identifier", context);
        case scanner::TokenType::Int:
            return formatter<string_view>::format("Int", context);
        case scanner::TokenType::None:
            return formatter<string_view>::format("None", context);
        case scanner::TokenType::String:
            return formatter<string_view>::format("String", context);
        case scanner::TokenType::True:
            return formatter<string_view>::format("True", context);
        case scanner::TokenType::EndOfFile:
            return formatter<string_view>::format("EndOfFile", context);
        case scanner::TokenType::Error:
            return formatter<string_view>::format("Error", context);
        default:
            POISE_UNREACHABLE();
    }
}
}   // namespace fmt
