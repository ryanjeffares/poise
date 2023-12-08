//
// Created by Ryan Jeffares on 07/12/2023.
//

#ifndef POISE_NATIVE_FUNCTION_HPP
#define POISE_NATIVE_FUNCTION_HPP

#include "../Poise.hpp"
#include "Value.hpp"

#include <type_traits>
#include <span>

namespace poise::runtime {
class NativeFunction
{
public:
    using Func = Value(*)(std::span<Value>);

    NativeFunction(u8 arity, Func function);

    [[nodiscard]] auto arity() const -> u8;
    [[nodiscard]] auto operator()(std::span<Value> args) const -> Value;

private:
    u8 m_arity;
    Func m_function;
};  // class NativeFunction
}   // namespace poise::runtime

#endif // POISE_NATIVE_FUNCTION_HPP
