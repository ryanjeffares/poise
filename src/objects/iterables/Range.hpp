//
// Created by ryand on 16/12/2023.
//

#ifndef POISE_RANGE_HPP
#define POISE_RANGE_HPP

#include "Iterable.hpp"

namespace poise::objects::iterables {
class Range : public Iterable
{
public:
    // all values are required to be checked to be numbers before this is called
    Range(const runtime::Value& start, const runtime::Value& end, const runtime::Value& increment, bool inclusive);
    ~Range() override = default;

    [[nodiscard]] auto asRange() noexcept -> Range* override;

    [[nodiscard]] auto toString() const noexcept -> std::string override;
    [[nodiscard]] auto type() const noexcept -> runtime::types::Type override;
    [[nodiscard]] auto iterable() const -> bool override;

    [[nodiscard]] auto begin() noexcept -> IteratorType override;
    [[nodiscard]] auto end() noexcept -> IteratorType override;
    auto incrementIterator(IteratorType& iterator) noexcept -> void override;
    auto isAtEnd(const IteratorType& iterator) noexcept -> bool override;
    [[nodiscard]] auto size() const noexcept -> usize override;
    [[nodiscard]] auto ssize() const noexcept -> isize override;
    auto unpack(std::vector<runtime::Value>& stack) const noexcept -> void override;

    [[nodiscard]] auto isInfiniteLoop() const noexcept -> bool;

    [[nodiscard]] auto rangeStart() const noexcept -> runtime::Value;
    [[nodiscard]] auto rangeEnd() const noexcept -> runtime::Value;
    [[nodiscard]] auto rangeIncrement() const noexcept -> runtime::Value;
    [[nodiscard]] auto rangeInclusive() const noexcept -> runtime::Value;

    [[nodiscard]] auto toVector() const noexcept -> std::vector<runtime::Value>;

private:
    static constexpr usize s_chunkSize = 8_uz;

    auto fillData(i64 value, i64 increment) -> void;

    bool m_inclusive;
    i64 m_start, m_end, m_increment;
    bool m_isInfiniteLoop{};
};
}   // namespace poise::objects::iterables

#endif  // #ifndef POISE_RANGE_HPP
