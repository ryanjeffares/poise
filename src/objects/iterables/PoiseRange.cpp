//
// Created by ryand on 16/12/2023.
//

#include "PoiseRange.hpp"
#include "../PoiseException.hpp"

#include <fmt/format.h>

namespace poise::objects::iterables {
PoiseRange::PoiseRange(runtime::Value start, runtime::Value end, runtime::Value increment)
    : m_start{std::move(start)}
    , m_end{std::move(end)}
    , m_increment{std::move(increment)}
{
    const auto useFloat = m_start.type() == runtime::types::Type::Float ||
        m_end.type() == runtime::types::Type::Float ||
        m_increment.type() == runtime::types::Type::Float;

    if (useFloat) {
        const auto s = m_start.toFloat();
        const auto e = m_end.toFloat();
        const auto i = m_increment.toFloat();

        if ((s < e) && (i > 0.0)) {
            // normal type of loop, start < end, increment > 0
            for (auto value = s; value < e; value += i) {
                m_data.emplace_back(value);
            }
        } else if ((e < s) && (i < 0.0)) {
            // reverse loop, start > end, increment < 0
            for (auto value = s; value > e; value += i) {
                m_data.emplace_back(value);
            }
        }
        // otherwise this is basically a noop
        // either the increment is 0, or going in the other direction of start -> end
        // so just don't fill the vector at all, begin == end, no iteration will happen if you try
    } else {
        const auto s = m_start.toInt();
        const auto e = m_end.toInt();
        const auto i = m_increment.toInt();

        if ((s < e) && (i > 0)) {
            // normal type of loop, start < end, increment > 0
            for (auto value = s; value < e; value += i) {
                m_data.emplace_back(value);
            }
        } else if ((e < s) && (i < 0)) {
            // reverse loop, start > end, increment < 0
            for (auto value = s; value > e; value += i) {
                m_data.emplace_back(value);
            }
        }
        // same logic as above
    }
}

auto PoiseRange::asRange() noexcept -> iterables::PoiseRange*
{
    return this;
}

auto PoiseRange::toString() const noexcept -> std::string
{
    return fmt::format("{}..{} by {}", m_start, m_end, m_increment);
}

auto PoiseRange::type() const noexcept -> runtime::types::Type
{
    return runtime::types::Type::Range;
}

auto PoiseRange::iterable() const -> bool
{
    return true;
}

auto PoiseRange::begin() noexcept -> PoiseIterable::IteratorType
{
    return m_data.begin();
}

auto PoiseRange::end() noexcept -> PoiseIterable::IteratorType
{
    return m_data.end();
}

auto PoiseRange::incrementIterator(PoiseIterable::IteratorType& iterator) noexcept -> void
{
    iterator++;
}

auto PoiseRange::isAtEnd(const PoiseIterable::IteratorType& iterator) noexcept -> bool
{
    return iterator == end();
}

auto PoiseRange::isInfiniteLoop() const noexcept -> bool
{
    return m_data.empty();
}
}   // namespace poise::objects::iterables