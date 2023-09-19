#ifndef POISE_OP_HPP
#define POISE_OP_HPP

#include <fmt/format.h>

#include <cstddef>
#include <cstdint>

namespace poise::runtime
{
    enum class Op : std::uint8_t
    {
        Call,
        DeclareFunction,
        Exit,
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

namespace fmt
{
    template<>
    struct formatter<poise::runtime::Op> : formatter<string_view>
    {
        auto format(poise::runtime::Op op, format_context& context) -> decltype(context.out());
    };
}

#endif
