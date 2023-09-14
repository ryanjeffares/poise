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
}

#endif
