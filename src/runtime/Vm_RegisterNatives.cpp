//
// Created by Ryan Jeffares on 07/12/2023.
//

#include "Vm.hpp"
#include "../objects/iterables/PoiseList.hpp"
#include "../objects/iterables/PoiseRange.hpp"
#include "../objects/PoiseException.hpp"
#include "Types.hpp"

#include <cmath>

namespace poise::runtime {
using objects::PoiseException;

auto throwIfWrongType(usize position, const Value& value, types::Type type) -> void
{
    if (value.type() != type) {
        throw PoiseException(PoiseException::ExceptionType::InvalidType, fmt::format("Expected {} at position {} but got {}", type, position, value.type()));
    }
}

auto throwIfWrongType(usize position, const Value& value, std::initializer_list<types::Type> types) -> void
{
    if (std::none_of(types.begin(), types.end(), [&value](types::Type type) {
        return value.type() == type;
    })) {
        throw PoiseException(PoiseException::ExceptionType::InvalidType, fmt::format("Expected {} at position {} but got {}", fmt::join(types, " or "), position, value.type()));
    }
}

auto Vm::registerNatives() noexcept -> void
{
    registerIntNatives();
    registerFloatNatives();
    registerListNatives();
    registerRangeNatives();
}   // Vm::registerNatives()

auto Vm::registerIntNatives() noexcept -> void
{
    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_INT_POW"), NativeFunction{
        2, [](std::span<Value> args) -> Value {
            throwIfWrongType(0, args[0], types::Type::Int);
            throwIfWrongType(1, args[1], types::Type::Int);
            return static_cast<i64>(std::pow(args[0].value<i64>(), args[1].value<i64>()));
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_INT_SQRT"), NativeFunction{
        1, [](std::span<Value> args) -> Value {
            throwIfWrongType(0, args[0], types::Type::Int);
            return static_cast<i64>(std::sqrt(args[0].value<i64>()));
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_INT_ABS"), NativeFunction{
        1, [](std::span<Value> args) -> Value {
            throwIfWrongType(0, args[0], types::Type::Int);
            return std::abs(args[0].value<i64>());
        }});
}

auto Vm::registerFloatNatives() noexcept -> void
{
    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_FLOAT_POW"), NativeFunction{
        2, [](std::span<Value> args) -> Value {
            throwIfWrongType(0, args[0], types::Type::Float);
            throwIfWrongType(1, args[1], {types::Type::Float, types::Type::Int});
            return static_cast<f64>(std::pow(args[0].value<f64>(), args[1].type() == types::Type::Float
                                                                   ? args[1].value<f64>()
                                                                   : static_cast<f64>(args[1].value<i64>())));
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_FLOAT_SQRT"), NativeFunction{
        1, [](std::span<Value> args) -> Value {
            throwIfWrongType(0, args[0], types::Type::Float);
            return static_cast<f64>(std::sqrt(args[0].value<f64>()));
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_FLOAT_ABS"), NativeFunction{
        1, [](std::span<Value> args) -> Value {
            throwIfWrongType(0, args[0], types::Type::Float);
            return std::abs(args[0].value<f64>());
        }});
}

auto Vm::registerListNatives() noexcept -> void
{
    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_LIST_APPEND"), NativeFunction{
        2, [](std::span<Value> args) -> Value {
            throwIfWrongType(0, args[0], types::Type::List);
            args[0].object()->asList()->append(std::move(args[1]));
            return Value::none();
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_LIST_INSERT"), NativeFunction{
        3, [](std::span<Value> args) -> Value {
            throwIfWrongType(0, args[0], types::Type::List);
            throwIfWrongType(1, args[1], types::Type::Int);
            return args[0].object()->asList()->insert(args[1].value<usize>(), std::move(args[2]));
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_LIST_REMOVE"), NativeFunction{
        2, [](std::span<Value> args) -> Value {
            throwIfWrongType(0, args[0], types::Type::List);
            return args[0].object()->asList()->remove(args[1]);
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_LIST_REMOVE_FIRST"), NativeFunction{
        2, [](std::span<Value> args) -> Value {
            throwIfWrongType(0, args[0], types::Type::List);
            return args[0].object()->asList()->removeFirst(args[1]);
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_LIST_REMOVE_AT"), NativeFunction{
        2, [](std::span<Value> args) -> Value {
            throwIfWrongType(0, args[0], types::Type::List);
            throwIfWrongType(1, args[1], types::Type::Int);
            return args[0].object()->asList()->removeAt(args[1].value<usize>());
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_LIST_CLEAR"), NativeFunction{
        1, [](std::span<Value> args) -> Value {
            throwIfWrongType(0, args[0], types::Type::List);
            args[0].object()->asList()->clear();
            return Value::none();
        }});
}

auto Vm::registerRangeNatives() noexcept -> void
{
    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_RANGE_IS_INFINITE_LOOP"), NativeFunction{
        1, [](std::span<Value> args) -> Value {
            throwIfWrongType(0, args[0], types::Type::Range);
            return args[0].object()->asRange()->isInfiniteLoop();
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_RANGE_START"), NativeFunction{
        1, [](std::span<Value> args) -> Value {
            throwIfWrongType(0, args[0], types::Type::Range);
            return args[0].object()->asRange()->rangeStart();
        }});
    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_RANGE_END"), NativeFunction{
        1, [](std::span<Value> args) -> Value {
            throwIfWrongType(0, args[0], types::Type::Range);
            return args[0].object()->asRange()->rangeEnd();
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_RANGE_INCREMENT"), NativeFunction{
        1, [](std::span<Value> args) -> Value {
            throwIfWrongType(0, args[0], types::Type::Range);
            return args[0].object()->asRange()->rangeIncrement();
        }});
}
}   // namespace poise::runtime
