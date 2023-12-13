//
// Created by ryand on 13/12/2023.
//

#ifndef POISE_ITERABLE_HPP
#define POISE_ITERABLE_HPP

#include "../../Poise.hpp"
#include "PoiseIterator.hpp"
#include "../PoiseObject.hpp"
#include "../../runtime/Value.hpp"

#include <span>

namespace poise::objects::iterables {
class PoiseIterable : public PoiseObject
{
public:
    using DifferenceType = std::vector<runtime::Value>::difference_type;
    using IteratorType = PoiseIterator::IteratorType;

    explicit PoiseIterable(usize initialSize, const runtime::Value& defaultValue = runtime::Value::none());
    explicit PoiseIterable(std::vector<runtime::Value> data);
    ~PoiseIterable() override;

    [[nodiscard]] virtual auto begin() noexcept -> IteratorType = 0;
    [[nodiscard]] virtual auto end() noexcept -> IteratorType = 0;
    virtual auto incrementIterator(IteratorType& iterator) noexcept -> void = 0;
    virtual auto isAtEnd(const IteratorType& iterator) noexcept -> bool = 0;

    auto addIterator(PoiseIterator* iterator) noexcept -> void;
    auto removeIterator(PoiseIterator* iterator) noexcept -> void;
    [[nodiscard]] auto data() const noexcept -> std::span<const runtime::Value>;

protected:
    auto invalidateIterators() noexcept -> void;

    std::vector<runtime::Value> m_data;
    std::vector<PoiseIterator*> m_activeIterators;
};
}   // namespace poise::objects::iterables

#endif  // #ifndef POISE_ITERABLE_HPP
