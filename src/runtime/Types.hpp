//
// Created by ryand on 13/12/2023.
//

#ifndef POISE_TYPES_HPP
#define POISE_TYPES_HPP

#include <fmt/format.h>

#include <unordered_map>

namespace poise::runtime {
class Value;

namespace types {
enum class Type
{
    // keep in alphabetical order - except anything that can't be constructed with a call
    // to its type ident should always be last (Type, Iterator, Pack)
    // so that we match this to the TokenType
    Bool, Float, Int, None, String, Exception, Function, List, Range, Type, Iterator,
};
}   // namespace types
}   // namespace poise::runtime

namespace fmt {
template<>
struct formatter<poise::runtime::types::Type> : formatter<string_view>
{
    [[nodiscard]] auto format(poise::runtime::types::Type type, format_context& context) const -> decltype(context.out());
};
}
#endif  // #ifndef POISE_TYPES_HPP
