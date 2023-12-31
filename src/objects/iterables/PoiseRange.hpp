//
// Created by ryand on 16/12/2023.
//

#ifndef POISE_RANGE_HPP
#define POISE_RANGE_HPP

#include "PoiseIterable.hpp"
#include "../PoiseObject.hpp"

namespace poise::objects::iterables {
class PoiseRange : public PoiseObject, public PoiseIterable
{
public:
    // all values are required to be checked to be numbers before this is called
    PoiseRange(runtime::Value start, runtime::Value end, runtime::Value increment, bool inclusive);
    ~PoiseRange() override = default;

    [[nodiscard]] auto asIterable() noexcept -> iterables::PoiseIterable* override;
    [[nodiscard]] auto asRange() noexcept -> iterables::PoiseRange* override;

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
    runtime::Value m_start, m_end, m_increment;
    bool m_isInfiniteLoop{};
};
}   // namespace poise::objects::iterables

#endif  // #ifndef POISE_RANGE_HPP
