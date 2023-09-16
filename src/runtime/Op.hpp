#ifndef POISE_OP_HPP
#define POISE_OP_HPP

#include <cstdint>

namespace poise::runtime
{
    enum class Op : std::uint8_t
    {
        Call,
        DeclareFunction,
        LoadConstant,
        PrintLn,
    };

    struct OpLine
    {
        Op op;
        std::size_t line;
    };
}

#endif
