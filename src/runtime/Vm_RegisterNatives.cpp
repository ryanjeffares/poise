//
// Created by Ryan Jeffares on 07/12/2023.
//

#include "Vm.hpp"
#include "../objects/PoiseException.hpp"

#include <cmath>

#define THROW_IF_WRONG_TYPE(expected, position)                                                                                                                                 \
    do {                                                                                                                                                                        \
        if (args[position].type() != (expected)) {                                                                                                                              \
            throw PoiseException(PoiseException::ExceptionType::InvalidType, fmt::format("Expected {} at position {} but got {}", expected, position, args[position].type()));  \
        }                                                                                                                                                                       \
    } while (false)


namespace poise::runtime {
using objects::PoiseException;

auto throwIfWrongType(usize position, const Value& value, Value::Type type) -> void
{
    if (value.type() != type) {
        throw PoiseException(PoiseException::ExceptionType::InvalidType, fmt::format("Expected {} at position {} but got {}", type, position, value.type()));
    }
}

auto throwIfWrongType(usize position, const Value& value, std::initializer_list<Value::Type> types) -> void
{
    if (std::none_of(types.begin(), types.end(), [&value] (Value::Type type) {
        return value.type() == type;
    })) {
        throw PoiseException(PoiseException::ExceptionType::InvalidType, fmt::format("Expected {} at position {} but got {}", fmt::join(types, " or "), position, value.type()));
    }
}

auto Vm::registerNatives() noexcept -> void
{
    registerIntNatives();
    registerFloatNatives();
}   // Vm::registerNatives()

auto Vm::registerIntNatives() noexcept -> void
{
    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_INT_POW"), NativeFunction{2, [] (std::span<Value> args) -> Value {
        throwIfWrongType(0, args[0], Value::Type::Int);
        throwIfWrongType(1, args[1], Value::Type::Int);
        return static_cast<i64>(std::pow(args[0].value<i64>(), args[1].value<i64>()));
    }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_INT_SQRT"), NativeFunction{1, [] (std::span<Value> args) -> Value {
        throwIfWrongType(0, args[0], Value::Type::Int);
        return static_cast<i64>(std::sqrt(args[0].value<i64>()));
    }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_INT_ABS"), NativeFunction{1, [] (std::span<Value> args) -> Value {
        throwIfWrongType(0, args[0], Value::Type::Int);
        return std::abs(args[0].value<i64>());
    }});
}

auto Vm::registerFloatNatives() noexcept -> void
{
    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_FLOAT_POW"), NativeFunction{2, [] (std::span<Value> args) -> Value {
        throwIfWrongType(0, args[0], Value::Type::Float);
        throwIfWrongType(1, args[1], {Value::Type::Float, Value::Type::Int});
        return static_cast<f64>(std::pow(args[0].value<f64>(), args[1].type() == Value::Type::Float ? args[1].value<f64>() : static_cast<f64>(args[1].value<i64>())));
    }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_FLOAT_SQRT"), NativeFunction{1, [] (std::span<Value> args) -> Value {
        THROW_IF_WRONG_TYPE(Value::Type::Float, 0);
        return static_cast<f64>(std::sqrt(args[0].value<f64>()));
    }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_FLOAT_ABS"), NativeFunction{1, [] (std::span<Value> args) -> Value {
        THROW_IF_WRONG_TYPE(Value::Type::Float, 0);
        return std::abs(args[0].value<f64>());
    }});
}
}   // namespace poise::runtime

#undef THROW_IF_WRONG_TYPE
