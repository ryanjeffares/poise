#ifndef TOKEN_TYPE_HPP
#define TOKEN_TYPE_HPP

#include "../Poise.hpp"

#include <fmt/format.h>

namespace poise::scanner {
enum class TokenType : u8
{
    // type identifiers - keep builtin types in this order at the very start
    BoolIdent,
    FloatIdent,
    IntIdent,
    NoneIdent,
    StringIdent,
    ExceptionIdent,
    FunctionIdent,

    // keywords
    And,
    As,
    Catch,
    Else,
    EPrint,
    EPrintLn,
    Export,
    Final,
    Func,
    If,
    Import,
    Or,
    Print,
    PrintLn,
    Return,
    This,
    Try,
    TypeOf,
    Var,
    While,

    // symbols
    Ampersand,
    Caret,
    CloseBrace,
    CloseParen,
    ColonColon,
    Comma,
    Dot,
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
}   // namespace poise::scanner

namespace fmt {
template<>
struct formatter<poise::scanner::TokenType> : formatter<string_view>
{
    auto format(poise::scanner::TokenType tokenType, format_context& context) const -> decltype(context.out());
};
}   // namespace fmt

#endif  // #ifndef TOKEN_TYPE_HPP