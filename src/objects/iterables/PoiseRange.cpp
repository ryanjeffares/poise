//
// Created by ryand on 16/12/2023.
//

#include "PoiseRange.hpp"
#include "../PoiseException.hpp"

#include <fmt/format.h>

namespace poise::objects::iterables {
PoiseRange::PoiseRange(runtime::Value start, runtime::Value end, runtime::Value increment, bool inclusive)
    : m_inclusive{inclusive}
    , m_start{std::move(start)}
    , m_end{std::move(end)}
    , m_increment{std::move(increment)}
    , m_useFloat{
        m_start.type() == runtime::types::Type::Float ||
        m_end.type() == runtime::types::Type::Float ||
        m_increment.type() == runtime::types::Type::Float
    }
{
    if (m_useFloat) {
        const auto s = m_start.toFloat();
        const auto e = m_end.toFloat();
        const auto i = m_increment.toFloat();

        if (((s < e) && (i > 0.0)) || ((e < s) && (i < 0.0))) {
            fillData(s, i);
        }
        // otherwise this is basically a noop
        // either the increment is 0, or going in the other direction of start -> end
        // so just don't fill the vector at all, begin == end, no iteration will happen if you try
    } else {
        const auto s = m_start.toInt();
        const auto e = m_end.toInt();
        const auto i = m_increment.toInt();

        if (((s < e) && (i > 0)) || ((e < s) && (i < 0))) {
            fillData(s, i);
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
    return fmt::format("{}{}{} by {}", m_start, m_inclusive ? "..=" : "..", m_end, m_increment);
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
    if (isAtEnd(iterator)) {
        // exhausted the current data, check if we need to refill
        if ((m_inclusive && m_data.back() == m_end) || (!m_inclusive && m_data.back() == m_end - m_increment)) {
            iterator = end();
        } else {
            if (m_useFloat) {
                auto value = m_data.back().toFloat() + m_increment.toFloat();
                fillData(value, m_increment.toFloat());
            } else {
                auto value = m_data.back().toInt() + m_increment.toInt();
                fillData(value, m_increment.toInt());
            }

            iterator = begin();
        }
    } else if ((m_inclusive && *iterator > m_end) || (!m_inclusive && *iterator >= m_end)) {
        // the actual value has gone past the end of the range, so make it as if we're at the end
        iterator = end();
    }
}

auto PoiseRange::isAtEnd(const PoiseIterable::IteratorType& iterator) noexcept -> bool
{
    return iterator == end();
}

auto PoiseRange::isInfiniteLoop() const noexcept -> bool
{
    if (m_useFloat) {
        const auto s = m_start.toFloat();
        const auto e = m_end.toFloat();
        const auto i = m_increment.toFloat();

        return i == 0.0 || (s < e && i < 0.0) || (s > e && i > 0.0);
    } else {
        const auto s = m_start.toInt();
        const auto e = m_end.toInt();
        const auto i = m_increment.toInt();

        return i == 0 || (s < e && i < 0) || (s > e && i > 0);
    }
}

auto PoiseRange::rangeStart() const noexcept -> runtime::Value
{
    return m_start;
}

auto PoiseRange::rangeEnd() const noexcept -> runtime::Value
{
    return m_end;
}

auto PoiseRange::rangeIncrement() const noexcept -> runtime::Value
{
    return m_increment;
}
}   // namespace poise::objects::iterables