//
// Created by ryand on 13/12/2023.
//

#ifndef POISE_ITERATOR_HPP
#define POISE_ITERATOR_HPP

#include "../../Poise.hpp"
#include "../PoiseObject.hpp"
#include "../../runtime/Value.hpp"

#include <vector>

namespace poise::objects::iterables {
class PoiseIterable;

class PoiseIterator : public PoiseObject
{
public:
    using IteratorType = std::vector<runtime::Value>::iterator;

    explicit PoiseIterator(runtime::Value iterable);
    ~PoiseIterator() override;

    [[nodiscard]] auto asIterator() noexcept -> PoiseIterator* override;

    [[nodiscard]] auto toString() const noexcept -> std::string override;
    [[nodiscard]] auto type() const noexcept -> runtime::types::Type override;

    auto increment() -> void;
    auto invalidate() noexcept -> void;
    [[nodiscard]] auto isAtEnd() const noexcept -> bool;
    [[nodiscard]] auto valid() const noexcept -> bool;
    [[nodiscard]] auto value() const -> const runtime::Value&;

private:
    auto throwIfInvalid() const -> void;

    runtime::Value m_iterableValue;
    PoiseIterable* m_iterablePtr;
    IteratorType m_iterator;
    bool m_isValid;
};
} // namespace poise::objects::iterables

#endif  // #ifndef POISE_ITERATOR_HPP