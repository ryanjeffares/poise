//
// Created by Ryan Jeffares on 07/12/2023.
//

#include "Vm.hpp"
#include "../objects/Objects.hpp"
#include "Types.hpp"

#include <fmt/ranges.h>

#include <cmath>

namespace poise::runtime {
using objects::Exception;

static auto throwIfWrongType(usize position, const Value& value, types::Type type) -> void
{
    if (value.type() != type) {
        throw Exception(Exception::ExceptionType::InvalidType, fmt::format("Expected {} at position {} but got {}", type, position, value.type()));
    }
}

static auto throwIfWrongType(usize position, const Value& value, std::initializer_list<types::Type> types) -> void
{
    if (std::none_of(types.begin(), types.end(), [&value](types::Type type) {
        return value.type() == type;
    })) {
        throw Exception(Exception::ExceptionType::InvalidType, fmt::format("Expected {} at position {} but got {}", fmt::join(types, " or "), position, value.type()));
    }
}

static auto throwIfNotIterable(usize position, const Value& value) -> void {
    if (value.object() == nullptr || value.object()->asIterable() == nullptr) {
        throw Exception(Exception::ExceptionType::InvalidType, fmt::format("Expected iterable at position {} but got {}", position, value.type()));
    }
}

auto Vm::registerNatives() noexcept -> void
{
    registerDictNatives();
    registerFloatNatives();
    registerIntNatives();
    registerIterableNatives();
    registerListNatives();
    registerSetNatives();
    registerRangeNatives();
    registerStringNatives();
}   // Vm::registerNatives()

auto Vm::registerDictNatives() noexcept -> void
{
    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_DICT_CONTAINS_KEY"), NativeFunction{
        2_u8, [](std::span<Value> args) -> Value {
            throwIfWrongType(0_uz, args[0_uz], types::Type::Dict);
            return args[0_uz].object()->asDictionary()->containsKey(args[1_uz]);
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_DICT_TRY_INSERT"), NativeFunction{
        3_u8, [](std::span<Value> args) -> Value {
            throwIfWrongType(0_uz, args[0_uz], types::Type::Dict);
            return args[0_uz].object()->asDictionary()->tryInsert(std::move(args[1_uz]), std::move(args[2_uz]));
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_DICT_INSERT"), NativeFunction{
        3_u8, [](std::span<Value> args) -> Value {
            throwIfWrongType(0_uz, args[0_uz], types::Type::Dict);
            args[0_uz].object()->asDictionary()->insertOrUpdate(std::move(args[1_uz]), std::move(args[2_uz]));
            return Value::none();
        }});
}

auto Vm::registerFloatNatives() noexcept -> void
{
    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_FLOAT_POW"), NativeFunction{
        2_u8, [](std::span<Value> args) -> Value {
            throwIfWrongType(0_uz, args[0_uz], types::Type::Float);
            throwIfWrongType(1_uz, args[1_uz], {types::Type::Float, types::Type::Int});
            return static_cast<f64>(std::pow(args[0_uz].value<f64>(), args[1_uz].type() == types::Type::Float
                                                                   ? args[1_uz].value<f64>()
                                                                   : static_cast<f64>(args[1_uz].value<i64>())));
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_FLOAT_SQRT"), NativeFunction{
        1_u8, [](std::span<Value> args) -> Value {
            throwIfWrongType(0_uz, args[0_uz], types::Type::Float);
            return static_cast<f64>(std::sqrt(args[0_uz].value<f64>()));
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_FLOAT_ABS"), NativeFunction{
        1_u8, [](std::span<Value> args) -> Value {
            throwIfWrongType(0_uz, args[0_uz], types::Type::Float);
            return std::abs(args[0_uz].value<f64>());
        }});
}

auto Vm::registerIntNatives() noexcept -> void
{
    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_INT_POW"), NativeFunction{
        2_u8, [](std::span<Value> args) -> Value {
            throwIfWrongType(0_uz, args[0_uz], types::Type::Int);
            throwIfWrongType(1_uz, args[1_uz], types::Type::Int);
            return static_cast<i64>(std::pow(args[0_uz].value<i64>(), args[1_uz].value<i64>()));
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_INT_SQRT"), NativeFunction{
        1_u8, [](std::span<Value> args) -> Value {
            throwIfWrongType(0_uz, args[0_uz], types::Type::Int);
            return static_cast<i64>(std::sqrt(args[0_uz].value<i64>()));
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_INT_ABS"), NativeFunction{
        1_u8, [](std::span<Value> args) -> Value {
            throwIfWrongType(0_uz, args[0_uz], types::Type::Int);
            return std::abs(args[0_uz].value<i64>());
        }});
}


auto Vm::registerIterableNatives() noexcept -> void
{
    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_ITERABLE_SIZE"), NativeFunction{
        1_u8, [](std::span<Value> args) -> Value {
            throwIfNotIterable(0_uz, args[0_uz]);
            return args[0_uz].object()->asIterable()->size();
        }});
}

auto Vm::registerListNatives() noexcept -> void
{
    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_LIST_EMPTY"), NativeFunction{
        1_u8, [](std::span<Value> args) -> Value {
            throwIfWrongType(0_uz, args[0_uz], types::Type::List);
            return args[0_uz].object()->asList()->empty();
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_LIST_APPEND"), NativeFunction{
        2_u8, [](std::span<Value> args) -> Value {
            throwIfWrongType(0_uz, args[0_uz], types::Type::List);
            args[0_uz].object()->asList()->append(std::move(args[1_uz]));
            return Value::none();
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_LIST_INSERT"), NativeFunction{
        3_u8, [](std::span<Value> args) -> Value {
            throwIfWrongType(0_uz, args[0_uz], types::Type::List);
            throwIfWrongType(1_uz, args[1_uz], types::Type::Int);
            return args[0_uz].object()->asList()->insert(args[1_uz].value<usize>(), std::move(args[2_uz]));
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_LIST_REMOVE"), NativeFunction{
        2_u8, [](std::span<Value> args) -> Value {
            throwIfWrongType(0_uz, args[0_uz], types::Type::List);
            return args[0_uz].object()->asList()->remove(args[1_uz]);
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_LIST_REMOVE_FIRST"), NativeFunction{
        2_u8, [](std::span<Value> args) -> Value {
            throwIfWrongType(0_uz, args[0_uz], types::Type::List);
            return args[0_uz].object()->asList()->removeFirst(args[1_uz]);
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_LIST_REMOVE_AT"), NativeFunction{
        2_u8, [](std::span<Value> args) -> Value {
            throwIfWrongType(0_uz, args[0_uz], types::Type::List);
            throwIfWrongType(1_uz, args[1_uz], types::Type::Int);
            return args[0_uz].object()->asList()->removeAt(args[1_uz].value<usize>());
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_LIST_CLEAR"), NativeFunction{
        1_u8, [](std::span<Value> args) -> Value {
            throwIfWrongType(0_uz, args[0_uz], types::Type::List);
            args[0_uz].object()->asList()->clear();
            return Value::none();
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_LIST_REPEAT"), NativeFunction{
        2_u8, [](std::span<Value> args) -> Value {
            throwIfWrongType(0_uz, args[0_uz], types::Type::List);
            throwIfWrongType(1_uz, args[1_uz], types::Type::Int);
            return args[0_uz].object()->asList()->repeat(args[1_uz].value<isize>());
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_LIST_CONCAT"), NativeFunction{
        2_u8, [](std::span<Value> args) -> Value {
            throwIfWrongType(0_uz, args[0_uz], types::Type::List);
            throwIfWrongType(1_uz, args[1_uz], types::Type::List);
            return args[0_uz].object()->asList()->concat(*args[1_uz].object()->asList());
        }});
}

auto Vm::registerSetNatives() noexcept -> void
{
    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_SET_INSERT"), NativeFunction{
        2_u8, [](std::span<Value> args) -> Value {
            throwIfWrongType(0_uz, args[0_uz], types::Type::Set);
            return args[0_uz].object()->asSet()->tryInsert(std::move(args[1_uz]));
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_SET_CONTAINS"), NativeFunction{
        2_u8, [](std::span<Value> args) -> Value {
            throwIfWrongType(0_uz, args[0_uz], types::Type::Set);
            return args[0_uz].object()->asSet()->contains(args[1_uz]);
        }});
}

auto Vm::registerRangeNatives() noexcept -> void
{
    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_RANGE_IS_INFINITE_LOOP"), NativeFunction{
        1_u8, [](std::span<Value> args) -> Value {
            throwIfWrongType(0_uz, args[0_uz], types::Type::Range);
            return args[0_uz].object()->asRange()->isInfiniteLoop();
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_RANGE_START"), NativeFunction{
        1_u8, [](std::span<Value> args) -> Value {
            throwIfWrongType(0_uz, args[0_uz], types::Type::Range);
            return args[0_uz].object()->asRange()->rangeStart();
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_RANGE_END"), NativeFunction{
        1_u8, [](std::span<Value> args) -> Value {
            throwIfWrongType(0_uz, args[0_uz], types::Type::Range);
            return args[0_uz].object()->asRange()->rangeEnd();
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_RANGE_INCREMENT"), NativeFunction{
        1_u8, [](std::span<Value> args) -> Value {
            throwIfWrongType(0_uz, args[0_uz], types::Type::Range);
            return args[0_uz].object()->asRange()->rangeIncrement();
        }});

    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_RANGE_INCLUSIVE"), NativeFunction{
        1_u8, [](std::span<Value> args) -> Value {
            throwIfWrongType(0_uz, args[0_uz], types::Type::Range);
            return args[0_uz].object()->asRange()->rangeInclusive();
        }});
}

auto Vm::registerStringNatives() noexcept -> void
{
    m_nativeFunctionLookup.emplace(m_nativeNameHasher("__NATIVE_STRING_LENGTH"), NativeFunction{
        1_u8, [](std::span<Value> args) -> Value {
            throwIfWrongType(0_uz, args[0_uz], types::Type::String);
            return args[0_uz].string().size();
        }});
}
}   // namespace poise::runtime

