//
// Created by Ryan Jeffares on 07/12/2023.
//

#include "Vm.hpp"
#include "../objects/PoiseException.hpp"

#include <cmath>

#define THROW_IF_WRONG_TYPE(expected, position)                                                                                                                                 \
    do {                                                                                                                                                                        \
        if (args[position].type() != expected) {                                                                                                                                \
            throw PoiseException(PoiseException::ExceptionType::InvalidType, fmt::format("Expected {} at position {} but got {}", expected, position, args[position].type())); \
        }                                                                                                                                                                       \
    } while (false)

namespace poise::runtime {
using objects::PoiseException;

auto Vm::registerNatives() noexcept -> void
{
    registerIntNatives();
}   // Vm::registerNatives()

auto Vm::registerIntNatives() noexcept -> void
{
    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_INT_POW"), NativeFunction{2, [] (std::span<Value> args) -> Value {
        THROW_IF_WRONG_TYPE(Value::Type::Int, 0);
        THROW_IF_WRONG_TYPE(Value::Type::Int, 1);
        return static_cast<i64>(std::pow(args[0].value<i64>(), args[1].value<i64>()));
    }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_INT_SQRT"), NativeFunction{1, [] (std::span<Value> args) -> Value {
        THROW_IF_WRONG_TYPE(Value::Type::Int, 0);
        return static_cast<i64>(std::sqrt(args[0].value<i64>()));
    }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_INT_ABS"), NativeFunction{1, [] (std::span<Value> args) -> Value {
        THROW_IF_WRONG_TYPE(Value::Type::Int, 0);
        return std::abs(args[0].value<i64>());
    }});
}
}   // namespace poise::runtime

#undef THROW_IF_WRONG_TYPE
