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
        case TokenType::TupleIdent:
            return true;
        default:
            return false;
    }
}

auto isGenericTypeIdent(TokenType tokenType) noexcept -> bool 
{
    switch (tokenType) {
        case TokenType::ListIdent:
        case TokenType::DictIdent:
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
           tokenType == TokenType::OpenSquareBracket;   // for lists
}

auto builtinGenericTypeCount(TokenType tokenType) noexcept -> u8
{
    POISE_ASSERT(isGenericTypeIdent(tokenType), fmt::format("Expected type ident that can be generic but got {}", tokenType));

    switch (tokenType) {
        case TokenType::ListIdent:
            return 1_u8;
        case TokenType::DictIdent:
            return 2_u8;
        default:
            return 0_u8;
    }
}

auto builtinConstructorAllowedArgCount(TokenType tokenType) noexcept -> AllowedArgCount
{
    POISE_ASSERT(isTypeIdent(tokenType), fmt::format("Expected type ident but got {}", tokenType));

    switch (tokenType) {   
        case TokenType::NoneIdent:
            return AllowedArgCount::None;
        case TokenType::ExceptionIdent:
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
    string_view result = "unknown";

    switch (tokenType) {
        case scanner::TokenType::BoolIdent:
            result = "BoolIdent";
            break;
        case scanner::TokenType::FloatIdent:
            result = "FloatIdent";
            break;
        case scanner::TokenType::IntIdent:
            result = "IntIdent";
            break;
        case scanner::TokenType::NoneIdent:
            result = "NoneIdent";
            break;
        case scanner::TokenType::StringIdent:
            result = "StringIdent";
            break;
        case scanner::TokenType::DictIdent:
            result = "DictIdent";
            break;
        case scanner::TokenType::ExceptionIdent:
            result = "ExceptionIdent";
            break;
        case scanner::TokenType::FunctionIdent:
            result = "FunctionIdent";
            break;
        case scanner::TokenType::ListIdent:
            result = "ListIdent";
            break;
        case scanner::TokenType::RangeIdent:
            result = "RangeIdent";
            break;
        case scanner::TokenType::TupleIdent:
            result = "TupleIdent";
            break;
        case scanner::TokenType::And:
            result = "And";
            break;
        case scanner::TokenType::As:
            result = "As";
            break;
        case scanner::TokenType::By:
            result = "By";
            break;
        case scanner::TokenType::Catch:
            result = "Catch";
            break;
        case scanner::TokenType::Else:
            result = "Else";
            break;
        case scanner::TokenType::EPrint:
            result = "EPrint";
            break;
        case scanner::TokenType::EPrintLn:
            result = "EPrintLn";
            break;
        case scanner::TokenType::Export:
            result = "Export";
            break;
        case scanner::TokenType::Final:
            result = "Final";
            break;
        case scanner::TokenType::For:
            result = "For";
            break;
        case scanner::TokenType::Func:
            result = "Func";
            break;
        case scanner::TokenType::If:
            result = "If";
            break;
        case scanner::TokenType::Import:
            result = "Import";
            break;
        case scanner::TokenType::In:
            result = "In";
            break;
        case scanner::TokenType::Or:
            result = "Or";
            break;
        case scanner::TokenType::Print:
            result = "Print";
            break;
        case scanner::TokenType::PrintLn:
            result = "PrintLn";
            break;
        case scanner::TokenType::Return:
            result = "Return";
            break;
        case scanner::TokenType::Throw:
            result = "Throw";
            break;
        case scanner::TokenType::This:
            result = "This";
            break;
        case scanner::TokenType::Try:
            result = "Try";
            break;
        case scanner::TokenType::TypeOf:
            result = "TypeOf";
            break;
        case scanner::TokenType::Var:
            result = "Var";
            break;
        case scanner::TokenType::While:
            result = "While";
            break;
        case scanner::TokenType::Ampersand:
            result = "Ampersand";
            break;
        case scanner::TokenType::Arrow:
            result = "Arrow";
            break;
        case scanner::TokenType::Caret:
            result = "Caret";
            break;
        case scanner::TokenType::CloseBrace:
            result = "CloseBrace";
            break;
        case scanner::TokenType::CloseParen:
            result = "CloseParen";
            break;
        case scanner::TokenType::CloseSquareBracket:
            result = "CloseSquareBracket";
            break;
        case scanner::TokenType::Colon:
            result = "Colon";
            break;
        case scanner::TokenType::ColonColon:
            result = "ColonColon";
            break;
        case scanner::TokenType::Comma:
            result = "Comma";
            break;
        case scanner::TokenType::Dot:
            result = "Dot";
            break;
        case scanner::TokenType::DotDot:
            result = "DotDot";
            break;
        case scanner::TokenType::DotDotDot:
            result = "DotDotDot";
            break;
        case scanner::TokenType::DotDotEqual:
            result = "DotDotEqual";
            break;
        case scanner::TokenType::Equal:
            result = "Equal";
            break;
        case scanner::TokenType::EqualEqual:
            result = "EqualEqual";
            break;
        case scanner::TokenType::Exclamation:
            result = "Exclamation";
            break;
        case scanner::TokenType::Greater:
            result = "Greater";
            break;
        case scanner::TokenType::GreaterEqual:
            result = "GreaterEqual";
            break;
        case scanner::TokenType::Less:
            result = "Less";
            break;
        case scanner::TokenType::LessEqual:
            result = "LessEqual";
            break;
        case scanner::TokenType::Modulus:
            result = "Modulus";
            break;
        case scanner::TokenType::Minus:
            result = "Subtraction";
            break;
        case scanner::TokenType::NotEqual:
            result = "NotEqual";
            break;
        case scanner::TokenType::OpenBrace:
            result = "OpenBrace";
            break;
        case scanner::TokenType::OpenParen:
            result = "OpenParen";
            break;
        case scanner::TokenType::OpenSquareBracket:
            result = "OpenSquareBracket";
            break;
        case scanner::TokenType::Pipe:
            result = "Pipe";
            break;
        case scanner::TokenType::Plus:
            result = "Addition";
            break;
        case scanner::TokenType::Semicolon:
            result = "Semicolon";
            break;
        case scanner::TokenType::ShiftLeft:
            result = "ShiftLeft";
            break;
        case scanner::TokenType::ShiftRight:
            result = "ShiftRight";
            break;
        case scanner::TokenType::Slash:
            result = "Slash";
            break;
        case scanner::TokenType::Star:
            result = "Star";
            break;
        case scanner::TokenType::Tilde:
            result = "Tilde";
            break;
        case scanner::TokenType::False:
            result = "False";
            break;
        case scanner::TokenType::Float:
            result = "Float";
            break;
        case scanner::TokenType::Identifier:
            result = "Identifier";
            break;
        case scanner::TokenType::Int:
            result = "Int";
            break;
        case scanner::TokenType::None:
            result = "None";
            break;
        case scanner::TokenType::String:
            result = "String";
            break;
        case scanner::TokenType::True:
            result = "True";
            break;
        case scanner::TokenType::EndOfFile:
            result = "EndOfFile";
            break;
        case scanner::TokenType::Error:
            result = "Error";
            break;
    }

    return formatter<string_view>::format(result, context);
}
}   // namespace fmt
