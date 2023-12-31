//
// Created by ryand on 13/12/2023.
//

#ifndef POISE_ITERABLE_HPP
#define POISE_ITERABLE_HPP

#include "../../Poise.hpp"
#include "PoiseIterator.hpp"
#include "../../runtime/Value.hpp"

#include <span>

namespace poise::objects::iterables {
class PoiseIterable
{
public:
    using IteratorType = PoiseIterator::IteratorType;
    using DifferenceType = std::vector<runtime::Value>::difference_type;

    PoiseIterable() = default;
    explicit PoiseIterable(usize initialSize, const runtime::Value& defaultValue = runtime::Value::none());
    explicit PoiseIterable(std::vector<runtime::Value> data);
    virtual ~PoiseIterable();

    [[nodiscard]] virtual auto begin() noexcept -> IteratorType = 0;
    [[nodiscard]] virtual auto end() noexcept -> IteratorType = 0;
    virtual auto incrementIterator(IteratorType& iterator) noexcept -> void = 0;
    virtual auto isAtEnd(const IteratorType& iterator) noexcept -> bool = 0;
    [[nodiscard]] virtual auto size() const noexcept -> usize = 0;
    [[nodiscard]] virtual auto ssize() const noexcept -> isize = 0;
    virtual auto unpack(std::vector<runtime::Value>& stack) const noexcept -> void = 0;

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
