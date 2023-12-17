//
// Created by ryand on 17/12/2023.
//

#include "PoisePack.hpp"

#include <fmt/format.h>

namespace poise::objects {
PoisePack::PoisePack(std::vector<runtime::Value> values) : m_values{std::move(values)}
{

}

PoisePack::PoisePack(std::span<runtime::Value> values) : m_values{values.begin(), values.end()}
{

}

auto PoisePack::asPack() noexcept -> PoisePack*
{
    return this;
}

auto PoisePack::toString() const noexcept -> std::string
{
    return fmt::format("{}", fmt::join(m_values, ", "));
}

auto PoisePack::type() const noexcept -> runtime::types::Type
{
    return runtime::types::Type::Pack;
}

auto PoisePack::values() const noexcept -> std::span<const runtime::Value>
{
    return m_values;
}

auto PoisePack::values() noexcept -> std::span<runtime::Value>
{
    return m_values;
}
}