//
// Created by ryand on 13/12/2023.
//

#ifndef POISE_TYPES_HPP
#define POISE_TYPES_HPP

#include "../Poise.hpp"

#include <fmt/format.h>

namespace poise::runtime {
class Value;

namespace types {
enum class Type
{
    // keep in alphabetical order - except anything that can't be constructed with a call
    // to its type ident should always be last (Type, Iterator, Pack)
    // so that we match this to the TokenType
    Bool, Float, Int, None, String, Dict, Exception, Function, List, Range, Set, Tuple, Type, Iterator,
};
}   // namespace types
}   // namespace poise::runtime

template<>
struct fmt::formatter<poise::runtime::types::Type> : formatter<string_view>
{
    [[nodiscard]] auto format(poise::runtime::types::Type type, format_context& context) const -> decltype(context.out());
};

#endif  // #ifndef POISE_TYPES_HPP
