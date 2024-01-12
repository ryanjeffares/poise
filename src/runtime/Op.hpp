#ifndef POISE_OP_HPP
#define POISE_OP_HPP

#include "../Poise.hpp"

#include <fmt/format.h>

namespace poise::runtime {
enum class Op : u8
{
    // stack/state modification
    AssignLocal,
    CaptureLocal,
    ConstructBuiltin,
    DeclareLocal,
    DeclareMultipleLocals,
    EnterTry,
    ExitTry,    // gracefully!
    LoadCapture,
    LoadConstant,
    LoadFunction,
    LoadLocal,
    LoadMember,
    LoadType,
    Pop,
    PopIterator,
    PopLocals,
    Throw,
    Unpack,

    // builtin functions
    TypeOf,

    // statements
    Print,

    // expressions
    LogicOr,
    LogicAnd,
    BitwiseOr,
    BitwiseXor,
    BitwiseAnd,
    Equal,
    NotEqual,
    LessThan,
    LessEqual,
    GreaterThan,
    GreaterEqual,
    LeftShift,
    RightShift,
    Addition,
    Subtraction,
    Multiply,
    Divide,
    Modulus,
    LogicNot,
    BitwiseNot,
    Negate,
    Plus,
    MakeLambda,
    AssignIndex,
    LoadIndex,

    // jumping/control flow
    Break,
    Call,
    CallNative,
    Exit,
    IncrementIterator,
    InitIterator,
    Jump,
    JumpIfFalse,
    JumpIfTrue,
    Return,
};

struct OpLine
{
    Op op;
    usize line;
};
}   // namespace poise::runtime

namespace fmt {
template<>
struct formatter<poise::runtime::Op> : formatter<string_view>
{
    [[nodiscard]] auto format(poise::runtime::Op op, format_context& context) -> decltype(context.out());
};
}   // namespace fmt

#endif  // #ifndef POISE_OP_HPP
