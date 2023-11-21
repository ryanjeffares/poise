#ifndef POISE_OP_HPP
#define POISE_OP_HPP

#include "../poise.hpp"

#include <fmt/format.h>

#include <cstddef>
#include <cstdint>

namespace poise::runtime
{
    enum class Op : u8
    {
        // stack/state modification
        DeclareFunction,
        LoadConstant,

        // statements
        PrintLn,

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
        Plus,
        Minus,
        Multiply,
        Divide,
        LogicNot,
        BitwiseNot,
        Negate,
        Pow,

        // jumping/control flow
        Call,
        Exit,
        Return,
    };

    struct OpLine
    {
        Op op;
        usize line;
    };
}

namespace fmt
{
    template<>
    struct formatter<poise::runtime::Op> : formatter<string_view>
    {
        auto format(poise::runtime::Op op, format_context& context) -> decltype(context.out());
    };
}

#endif
