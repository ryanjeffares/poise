//
// Created by ryand on 16/12/2023.
//

#include "Range.hpp"

#include <fmt/format.h>

namespace poise::objects::iterables {
Range::Range(const runtime::Value& start, const runtime::Value& end, const runtime::Value& increment, bool inclusive)
    : m_inclusive{inclusive}
    , m_start{start.value<i64>()}
    , m_end{end.value<i64>()}
    , m_increment{increment.value<i64>()}
{
    if ((m_start < m_end && m_increment > 0) || (m_end < m_start && m_increment < 0)) {
        fillData(m_start, m_increment);
    } else {
        // otherwise no iteration is possible
        // either the increment is 0, or going in the other direction of start -> end
        // so just don't fill the vector at all, begin == end, no iteration will happen if you try
        m_isInfiniteLoop = true;
    }
}

auto Range::asRange() noexcept -> Range*
{
    return this;
}

auto Range::toString() const noexcept -> std::string
{
    return fmt::format("{}{}{} by {}", m_start, m_inclusive ? "..=" : "..", m_end, m_increment);
}

auto Range::type() const noexcept -> runtime::types::Type
{
    return runtime::types::Type::Range;
}

auto Range::iterable() const -> bool
{
    return true;
}

auto Range::size() const noexcept -> usize
{
    if (m_isInfiniteLoop) {
        return 0_uz;
    }

    const auto range = m_start < m_end ? std::abs(m_end - m_start) : std::abs(m_start - m_end);
    return static_cast<usize>(range / std::abs(m_increment));
}

auto Range::ssize() const noexcept -> isize
{
    return static_cast<isize>(size());
}

auto Range::unpack(std::vector<runtime::Value>& stack) const noexcept -> void
{
    if (m_isInfiniteLoop) {
        stack.emplace_back(0);
        stack.emplace_back(0);
        return;
    }

    const auto upwards = m_end > m_start;
    auto current = m_start;

    for (;; current += m_increment) {
        if (upwards ? (m_inclusive ? current > m_end : current >= m_end) : (m_inclusive ? current < m_end : current <= m_end)) {
            break;
        }

        stack.emplace_back(current);
    }

    stack.emplace_back(size());
}

auto Range::begin() noexcept -> IteratorType
{
    return m_data.begin();
}

auto Range::end() noexcept -> IteratorType
{
    return m_data.end();
}

auto Range::incrementIterator(IteratorType& iterator) noexcept -> void
{
    ++iterator;

    if (isAtEnd(iterator)) {
        // exhausted the current data, check if we need to refill
        if (!(m_inclusive && m_data.back() == m_end) && !(!m_inclusive && m_data.back() == m_end - m_increment)) {
            std::vector<DifferenceType> iteratorIndexes;
            iteratorIndexes.reserve(m_activeIterators.size());
            for (auto i = 0_uz; i < m_activeIterators.size(); i++) {
                iteratorIndexes.push_back(std::distance(m_data.begin(), m_activeIterators[i]->iterator()));
            }

            const auto value = m_data.back().toInt() + m_increment;
            fillData(value, m_increment);

            for (auto i = 0_uz; i < m_activeIterators.size(); i++) {
                m_activeIterators[i]->iterator() = m_data.begin() + iteratorIndexes[i];
            }
        }
    } else if ((m_inclusive && *iterator > m_end) || (!m_inclusive && *iterator >= m_end)) {
        // the actual value has gone past the end of the range, so make it as if we're at the end
        iterator = end();
    }
}

auto Range::isAtEnd(const IteratorType& iterator) noexcept -> bool
{
    return iterator == end();
}

auto Range::isInfiniteLoop() const noexcept -> bool
{
    return m_isInfiniteLoop;
}

auto Range::rangeStart() const noexcept -> runtime::Value
{
    return m_start;
}

auto Range::rangeEnd() const noexcept -> runtime::Value
{
    return m_end;
}

auto Range::rangeIncrement() const noexcept -> runtime::Value
{
    return m_increment;
}

auto Range::rangeInclusive() const noexcept -> runtime::Value
{
    return m_inclusive;
}

auto Range::fillData(i64 value, i64 increment) -> void
{
    const auto size = m_data.size();
    m_data.resize(size + s_chunkSize);

    for (auto i = size; i < m_data.size(); i++) {
        m_data[i] = value;
        value += increment;
    }
}

auto Range::toVector() const noexcept -> std::vector<runtime::Value>
{
    if (m_isInfiniteLoop) {
        return {};
    }

    std::vector<runtime::Value> res;

    const auto upwards = m_end > m_start;
    auto current = m_start;

    for (;; current += m_increment) {
        if (upwards ? (m_inclusive ? current > m_end : current >= m_end) : (m_inclusive ? current < m_end : current <= m_end)) {
            break;
        }

        res.emplace_back(current);
    }

    return res;
}
}   // namespace poise::objects::iterables
 
