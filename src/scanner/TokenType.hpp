#ifndef TOKEN_TYPE_HPP
#define TOKEN_TYPE_HPP

#include "../Poise.hpp"

#include <fmt/format.h>

#include <string_view>

namespace poise::scanner {
enum class TokenType : u8
{
    // type identifiers - keep builtin types in this order at the very start
    BoolIdent,
    FloatIdent,
    IntIdent,
    NoneIdent,
    StringIdent,
    DictIdent,
    ExceptionIdent,
    FunctionIdent,
    ListIdent,
    RangeIdent,
    SetIdent,
    TupleIdent,

    // keywords
    And,
    As,
    Break,
    By,
    Catch,
    Const,
    Continue,
    Else,
    EPrint,
    EPrintLn,
    Export,
    Final,
    For,
    Func,
    If,
    Import,
    In,
    Or,
    Print,
    PrintLn,
    Return,
    This,
    Throw,
    Try,
    TypeOf,
    Var,
    While,

    // symbols
    Ampersand,
    Arrow,
    Caret,
    CloseBrace,
    CloseParen,
    CloseSquareBracket,
    Colon,
    ColonColon,
    Comma,
    Dot,
    DotDot,
    DotDotDot,
    DotDotEqual,
    Equal,
    EqualEqual,
    Exclamation,
    Greater,
    GreaterEqual,
    Less,
    LessEqual,
    Minus,
    Modulus,
    NotEqual,
    OpenBrace,
    OpenParen,
    OpenSquareBracket,
    Pipe,
    Plus,
    Semicolon,
    ShiftLeft,
    ShiftRight,
    Slash,
    Star,
    Tilde,

    // literals
    False,
    Float,
    Identifier,
    Int,
    None,
    String,
    True,

    // other
    EndOfFile,
    Error,
};

[[nodiscard]] auto isLiteral(TokenType tokenType) noexcept -> bool;
[[nodiscard]] auto isUnaryOp(TokenType tokenType) noexcept -> bool;
[[nodiscard]] auto isTypeIdent(TokenType tokenType) noexcept -> bool;
[[nodiscard]] auto isPrimitiveTypeIdent(TokenType tokenType) noexcept -> bool;
[[nodiscard]] auto isBuiltinFunction(TokenType tokenType) noexcept -> bool;
[[nodiscard]] auto isValidStartOfExpression(TokenType tokenType) noexcept -> bool;

[[nodiscard]] auto typeDisplayName(TokenType tokenType) noexcept -> std::string_view;

enum class AllowedGenericTypeCount
{
    None, One, Two, Any,
};

[[nodiscard]] auto builtinGenericTypeCount(TokenType tokenType) noexcept -> AllowedGenericTypeCount;

// for the record, i hate this
enum class AllowedArgCount
{
    None, One, OneOrNone, OneOrMore, TwoOrThree, Any,
};

[[nodiscard]] auto builtinConstructorAllowedArgCount(TokenType tokenType) noexcept -> AllowedArgCount;
}   // namespace poise::scanner

template<>
struct fmt::formatter<poise::scanner::TokenType> : formatter<string_view>
{
    auto format(poise::scanner::TokenType tokenType, format_context& context) const -> decltype(context.out());
};   // namespace fmt

#endif  // #ifndef TOKEN_TYPE_HPP
