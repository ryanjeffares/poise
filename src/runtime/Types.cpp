//
// Created by ryand on 13/12/2023.
//

#include "Types.hpp"

namespace fmt {
using poise::runtime::types::Type;

auto formatter<Type>::format(Type type, format_context& context) const -> decltype(context.out())
{
    switch (type) {
        case Type::Bool:
            return formatter<string_view>::format("Bool", context);
        case Type::Float:
            return formatter<string_view>::format("Float", context);
        case Type::Int:
            return formatter<string_view>::format("Int", context);
        case Type::None:
            return formatter<string_view>::format("None", context);
        case Type::String:
            return formatter<string_view>::format("String", context);
        case Type::Dict:
            return formatter<string_view>::format("Dict", context);
        case Type::Exception:
            return formatter<string_view>::format("Exception", context);
        case Type::Function:
            return formatter<string_view>::format("Function", context);
        case Type::Iterator:
            return formatter<string_view>::format("Iterator", context);
        case Type::List:
            return formatter<string_view>::format("List", context);
        case Type::Range:
            return formatter<string_view>::format("Range", context);
        case Type::Set:
            return formatter<string_view>::format("Set", context);
        case Type::Struct:
            return formatter<string_view>::format("Struct", context);
        case Type::Tuple:
            return formatter<string_view>::format("Tuple", context);
        case Type::Type:
            return formatter<string_view>::format("Type", context);
        default:
            POISE_UNREACHABLE();
    }
}
}   // namespace fmt
