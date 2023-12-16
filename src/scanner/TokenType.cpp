#include "TokenType.hpp"

namespace poise::scanner {
auto isLiteral(TokenType tokenType) noexcept -> bool
{
    return tokenType == TokenType::False ||
           tokenType == TokenType::Float ||
           tokenType == TokenType::Identifier ||
           tokenType == TokenType::Int ||
           tokenType == TokenType::None ||
           tokenType == TokenType::String ||
           tokenType == TokenType::True;
}

auto isUnaryOp(TokenType tokenType) noexcept -> bool
{
    return tokenType == TokenType::Exclamation ||
           tokenType == TokenType::Tilde ||
           tokenType == TokenType::Minus ||
           tokenType == TokenType::Plus;
}

auto isTypeIdent(TokenType tokenType) noexcept -> bool
{
    return tokenType == TokenType::BoolIdent ||
           tokenType == TokenType::FloatIdent ||
           tokenType == TokenType::IntIdent ||
           tokenType == TokenType::NoneIdent ||
           tokenType == TokenType::StringIdent ||
           tokenType == TokenType::ExceptionIdent ||
           tokenType == TokenType::FunctionIdent ||
           tokenType == TokenType::ListIdent ||
           tokenType == TokenType::RangeIdent;
}

auto isPrimitiveTypeIdent(TokenType tokenType) noexcept -> bool
{
    return tokenType == TokenType::BoolIdent ||
           tokenType == TokenType::FloatIdent ||
           tokenType == TokenType::IntIdent ||
           tokenType == TokenType::NoneIdent ||
           tokenType == TokenType::StringIdent;
}

auto isBuiltinFunction(TokenType tokenType) noexcept -> bool
{
    return tokenType == TokenType::TypeOf;
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
}   // namespace poise::scanner

namespace fmt {
using namespace poise::scanner;

auto formatter<TokenType>::format(TokenType tokenType, format_context& context) const -> decltype(context.out())
{
    string_view result = "unknown";

    switch (tokenType) {
        case TokenType::BoolIdent:
            result = "BoolIdent";
            break;
        case TokenType::FloatIdent:
            result = "FloatIdent";
            break;
        case TokenType::IntIdent:
            result = "IntIdent";
            break;
        case TokenType::NoneIdent:
            result = "NoneIdent";
            break;
        case TokenType::StringIdent:
            result = "StringIdent";
            break;
        case TokenType::ExceptionIdent:
            result = "ExceptionIdent";
            break;
        case TokenType::FunctionIdent:
            result = "FunctionIdent";
            break;
        case TokenType::ListIdent:
            result = "ListIdent";
            break;
        case TokenType::RangeIdent:
            result = "RangeIdent";
            break;
        case TokenType::And:
            result = "And";
            break;
        case TokenType::As:
            result = "As";
            break;
        case TokenType::By:
            result = "By";
            break;
        case TokenType::Catch:
            result = "Catch";
            break;
        case TokenType::Else:
            result = "Else";
            break;
        case TokenType::EPrint:
            result = "EPrint";
            break;
        case TokenType::EPrintLn:
            result = "EPrintLn";
            break;
        case TokenType::Export:
            result = "Export";
            break;
        case TokenType::Final:
            result = "Final";
            break;
        case TokenType::For:
            result = "For";
            break;
        case TokenType::Func:
            result = "Func";
            break;
        case TokenType::If:
            result = "If";
            break;
        case TokenType::Import:
            result = "Import";
            break;
        case TokenType::In:
            result = "In";
            break;
        case TokenType::Or:
            result = "Or";
            break;
        case TokenType::Print:
            result = "Print";
            break;
        case TokenType::PrintLn:
            result = "PrintLn";
            break;
        case TokenType::Return:
            result = "Return";
            break;
        case TokenType::This:
            result = "This";
            break;
        case TokenType::Try:
            result = "Try";
            break;
        case TokenType::TypeOf:
            result = "TypeOf";
            break;
        case TokenType::Var:
            result = "Var";
            break;
        case TokenType::While:
            result = "While";
            break;
        case TokenType::Ampersand:
            result = "Ampersand";
            break;
        case TokenType::Caret:
            result = "Caret";
            break;
        case TokenType::CloseBrace:
            result = "CloseBrace";
            break;
        case TokenType::CloseParen:
            result = "CloseParen";
            break;
        case TokenType::CloseSquareBracket:
            result = "CloseSquareBracket";
            break;
        case TokenType::ColonColon:
            result = "ColonColon";
            break;
        case TokenType::Comma:
            result = "Comma";
            break;
        case TokenType::Dot:
            result = "Dot";
            break;
        case TokenType::DotDot:
            result = "DotDot";
            break;
        case TokenType::DotDotEqual:
            result = "DotDotEqual";
            break;
        case TokenType::Equal:
            result = "Equal";
            break;
        case TokenType::EqualEqual:
            result = "EqualEqual";
            break;
        case TokenType::Exclamation:
            result = "Exclamation";
            break;
        case TokenType::Greater:
            result = "Greater";
            break;
        case TokenType::GreaterEqual:
            result = "GreaterEqual";
            break;
        case TokenType::Less:
            result = "Less";
            break;
        case TokenType::LessEqual:
            result = "LessEqual";
            break;
        case TokenType::Modulus:
            result = "Modulus";
            break;
        case TokenType::Minus:
            result = "Subtraction";
            break;
        case TokenType::NotEqual:
            result = "NotEqual";
            break;
        case TokenType::OpenBrace:
            result = "OpenBrace";
            break;
        case TokenType::OpenParen:
            result = "OpenParen";
            break;
        case TokenType::OpenSquareBracket:
            result = "OpenSquareBracket";
            break;
        case TokenType::Pipe:
            result = "Pipe";
            break;
        case TokenType::Plus:
            result = "Addition";
            break;
        case TokenType::Semicolon:
            result = "Semicolon";
            break;
        case TokenType::ShiftLeft:
            result = "ShiftLeft";
            break;
        case TokenType::ShiftRight:
            result = "ShiftRight";
            break;
        case TokenType::Slash:
            result = "Slash";
            break;
        case TokenType::Star:
            result = "Star";
            break;
        case TokenType::Tilde:
            result = "Tilde";
            break;
        case TokenType::False:
            result = "False";
            break;
        case TokenType::Float:
            result = "Float";
            break;
        case TokenType::Identifier:
            result = "Identifier";
            break;
        case TokenType::Int:
            result = "Int";
            break;
        case TokenType::None:
            result = "None";
            break;
        case TokenType::String:
            result = "String";
            break;
        case TokenType::True:
            result = "True";
            break;
        case TokenType::EndOfFile:
            result = "EndOfFile";
            break;
        case TokenType::Error:
            result = "Error";
            break;
    }

    return formatter<string_view>::format(result, context);
}
}   // namespace fmt