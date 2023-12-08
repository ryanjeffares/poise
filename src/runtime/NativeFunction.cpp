//
// Created by Ryan Jeffares on 08/12/2023.
//

#include "NativeFunction.hpp"

namespace poise::runtime {
NativeFunction::NativeFunction(u8 arity, Func function)
    : m_arity{arity}
    , m_function{function}
{

}

auto NativeFunction::arity() const noexcept -> u8
{
    return m_arity;
}

auto NativeFunction::operator()(std::span<Value> args) const -> Value
{
    return m_function(args);
}
}   // namespace poise::runtime
