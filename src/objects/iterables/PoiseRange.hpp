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
    PoiseRange(runtime::Value start, runtime::Value end, runtime::Value increment);
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

private:
    runtime::Value m_start, m_end, m_increment;
};
}   // namespace poise::objects::iterables

#endif  // #ifndef POISE_RANGE_HPP
