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
{
    const auto s = m_start.toInt();
    const auto e = m_end.toInt();
    const auto i = m_increment.toInt();

    if (((s < e) && (i > 0)) || ((e < s) && (i < 0))) {
        fillData(s, i);
    } else {
        // otherwise no iteration is possible
        // either the increment is 0, or going in the other direction of start -> end
        // so just don't fill the vector at all, begin == end, no iteration will happen if you try
        m_isInfiniteLoop = true;
    }
}

auto PoiseRange::asIterable() noexcept -> iterables::PoiseIterable*
{
    return this;
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

auto PoiseRange::size() const noexcept -> usize
{
    if (m_isInfiniteLoop) {
        return 0_uz;
    }

    const auto s = m_start.toInt();
    const auto e = m_end.toInt();
    const auto i = std::abs(m_increment.toInt());

    const auto range = s < e ? std::abs(e - s) : std::abs(s - e);
    return static_cast<usize>(range / i);
}

auto PoiseRange::ssize() const noexcept -> isize
{
    return static_cast<isize>(size());
}

auto PoiseRange::unpack(std::vector<runtime::Value>& stack) const noexcept -> void
{
    if (m_isInfiniteLoop) {
        stack.emplace_back(0);
        return;
    }

    auto s = m_start.toInt();
    const auto e = m_end.toInt();
    const auto inc = m_increment.toInt();
    const auto upwards = e > s;

    for (;; s += inc) {
        if (upwards ? (m_inclusive ? (s > e) : (s >= e)) : (m_inclusive ? (s < e) : (s <= e))) {
            break;
        }

        stack.emplace_back(s);
    }

    stack.emplace_back(size());
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
        if (!(m_inclusive && m_data.back() == m_end) && !(!m_inclusive && m_data.back() == m_end - m_increment)) {
            std::vector<DifferenceType> iteratorIndexes;
            iteratorIndexes.reserve(m_activeIterators.size());
            for (auto i = 0_uz; i < m_activeIterators.size(); i++) {
                iteratorIndexes.push_back(std::distance(m_data.begin(), m_activeIterators[i]->iterator()));
            }

            auto value = m_data.back().toInt() + m_increment.toInt();
            fillData(value, m_increment.toInt());

            for (auto i = 0_uz; i < m_activeIterators.size(); i++) {
                m_activeIterators[i]->iterator() = m_data.begin() + iteratorIndexes[i];
            }
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
    return m_isInfiniteLoop;
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

auto PoiseRange::rangeInclusive() const noexcept -> runtime::Value
{
    return m_inclusive;
}

auto PoiseRange::fillData(i64 value, i64 increment) -> void
{
    const auto size = m_data.size();
    m_data.resize(size + s_chunkSize);

    for (auto i = size; i < m_data.size(); i++) {
        m_data[i] = value;
        value += increment;
    }
}

auto PoiseRange::toVector() const noexcept -> std::vector<runtime::Value>
{
    if (m_isInfiniteLoop) {
        return {};
    }

    std::vector<runtime::Value> res;

    auto s = m_start.toInt();
    const auto e = m_end.toInt();
    const auto inc = m_increment.toInt();
    const auto upwards = e > s;

    for (;; s += inc) {
        if (upwards ? (m_inclusive ? (s > e) : (s >= e)) : (m_inclusive ? (s < e) : (s <= e))) {
            break;
        }

        res.emplace_back(s);
    }

    return res;
}
}   // namespace poise::objects::iterables
