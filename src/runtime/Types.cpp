//
// Created by ryand on 13/12/2023.
//

#include "Types.hpp"
#include "Value.hpp"
#include "../objects/PoiseType.hpp"

namespace poise::runtime::types {
static const std::unordered_map <Type, runtime::Value> s_typeLookup = {
    {types::Type::Bool, Value::createObject<objects::PoiseType>(Type::Bool, "Bool")},
    {types::Type::Float, Value::createObject<objects::PoiseType>(Type::Float, "Float")},
    {types::Type::Int, Value::createObject<objects::PoiseType>(Type::Int, "Int")},
    {types::Type::None, Value::createObject<objects::PoiseType>(Type::None, "None")},
    {types::Type::String, Value::createObject<objects::PoiseType>(Type::String, "String")},
    {types::Type::Exception, Value::createObject<objects::PoiseType>(Type::Exception, "Exception")},
    {types::Type::Function, Value::createObject<objects::PoiseType>(Type::Function, "Function")},
    {types::Type::Iterator, Value::createObject<objects::PoiseType>(Type::Iterator, "Iterator")},
    {types::Type::List, Value::createObject<objects::PoiseType>(Type::List, "List")},
    {types::Type::Range, Value::createObject<objects::PoiseType>(Type::Range, "Range")},
    {types::Type::Type, Value::createObject<objects::PoiseType>(Type::Type, "TypeInternal")},
};

auto typeValue(Type type) noexcept -> const runtime::Value&
{
    return s_typeLookup.at(type);
}
}   // namespace poise::runtime::types

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
        case Type::Type:
            res = "TypeInternal";
            break;
    }

    return formatter<string_view>::format(res, context);
}
}   // namespace fmt
