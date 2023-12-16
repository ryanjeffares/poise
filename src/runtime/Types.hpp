//
// Created by ryand on 13/12/2023.
//

#ifndef POISE_TYPES_HPP
#define POISE_TYPES_HPP

#include <fmt/format.h>

namespace poise::runtime {
class Value;

namespace types {
enum class Type
{
    // keep in alphabetical order - except Iterator should always be last
    // so that we match this to the TokenType
    Bool, Float, Int, None, String, Exception, Function, List, Range, Type, Iterator,
};

auto typeValue(Type type) noexcept -> const runtime::Value&;
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
