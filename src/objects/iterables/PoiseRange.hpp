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
    PoiseRange(runtime::Value value, runtime::Value end, runtime::Value increment, bool inclusive);
    ~PoiseRange() override = default;

    [[nodiscard]] auto asRange() noexcept -> iterables::PoiseRange* override;

    [[nodiscard]] auto toString() const noexcept -> std::string override;
    [[nodiscard]] auto type() const noexcept -> runtime::types::Type override;
    [[nodiscard]] auto iterable() const -> bool override;

    [[nodiscard]] auto begin() noexcept -> IteratorType override;
    [[nodiscard]] auto end() noexcept -> IteratorType override;
    auto incrementIterator(IteratorType& iterator) noexcept -> void override;
    auto isAtEnd(const IteratorType& iterator) noexcept -> bool override;

    [[nodiscard]] auto isInfiniteLoop() const noexcept -> bool;

    [[nodiscard]] auto rangeStart() const noexcept -> runtime::Value;
    [[nodiscard]] auto rangeEnd() const noexcept -> runtime::Value;
    [[nodiscard]] auto rangeIncrement() const noexcept -> runtime::Value;

private:
    static constexpr usize s_capacity = 8_uz;

    template<typename T>
    requires(std::is_same_v<T, f64> || std::is_same_v<T, i64>)
    auto fillData(T value, T increment) -> void
    {
        m_data.resize(s_capacity);

        for (auto i = 0_uz; i < m_data.size(); i++) {
            m_data[i] = value;
            value += increment;
        }
    }

    bool m_inclusive;
    runtime::Value m_start, m_end, m_increment;
    bool m_useFloat;
};
}   // namespace poise::objects::iterables

#endif  // #ifndef POISE_RANGE_HPP
