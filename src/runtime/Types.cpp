//
// Created by ryand on 13/12/2023.
//

#include "Types.hpp"

namespace fmt {
using poise::runtime::types::Type;

auto formatter<Type>::format(Type type, format_context& context) const -> decltype(context.out())
{
    string_view res = "unknown";

    switch (type) {
        case Type::Bool:
            res = "Bool";
            break;
        case Type::Float:
            res = "Float";
            break;
        case Type::Int:
            res = "Int";
            break;
        case Type::None:
            res = "None";
            break;
        case Type::String:
            res = "String";
            break;
        case Type::Exception:
            res = "Exception";
            break;
        case Type::Function:
            res = "Function";
            break;
        case Type::Iterator:
            res = "Iterator";
            break;
        case Type::List:
            res = "List";
            break;
        case Type::Range:
            res = "Range";
            break;
        case Type::Tuple:
            res = "Tuple";
            break;
        case Type::Type:
            res = "Type";
            break;
    }

    return formatter<string_view>::format(res, context);
}
}   // namespace fmt
