#ifndef POISE_OP_HPP
#define POISE_OP_HPP

#include <cstddef>
#include <cstdint>

namespace poise::runtime
{
    enum class Op : std::uint8_t
    {
        Call,
        DeclareFunction,
        LoadConstant,
        PrintLn,
        Return,
    };

    struct OpLine
    {
        Op op;
        std::size_t line;
    };
}

#endif
