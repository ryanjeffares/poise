//
// Created by ryand on 13/12/2023.
//

#ifndef POISE_ITERATOR_HPP
#define POISE_ITERATOR_HPP

#include "../Object.hpp"
#include "../../runtime/Value.hpp"

#include <vector>

namespace poise::objects::iterables {
class Iterable;

class Iterator : public Object
{
public:
    using IteratorType = std::vector<runtime::Value>::iterator;
    using DifferenceType = IteratorType::difference_type;

    explicit Iterator(runtime::Value iterable);
    // only for testing
    explicit Iterator(Iterable* iterable);

    ~Iterator() override;

    [[nodiscard]] auto asIterator() noexcept -> Iterator* override;

    [[nodiscard]] auto toString() const noexcept -> std::string override;
    [[nodiscard]] auto type() const noexcept -> runtime::types::Type override;
    auto findObjectMembers(std::vector<Object*>& objects) const noexcept -> void override;
    auto removeObjectMembers() noexcept -> void override;

    auto increment() -> void;
    auto invalidate() noexcept -> void;
    [[nodiscard]] auto isAtEnd() const noexcept -> bool;
    [[nodiscard]] auto valid() const noexcept -> bool;
    [[nodiscard]] auto iterator() const noexcept -> const IteratorType&;
    [[nodiscard]] auto iterator() noexcept -> IteratorType&;
    [[nodiscard]] auto value() const -> const runtime::Value&;
    [[nodiscard]] auto iterableValue() const noexcept -> const runtime::Value&;
    [[nodiscard]] auto iterablePtr() const noexcept -> Iterable*;

private:
    auto throwIfInvalid() const -> void;

    runtime::Value m_iterableValue;
    Iterable* m_iterablePtr;
    IteratorType m_iterator;
    bool m_isValid;
};
} // namespace poise::objects::iterables

#endif  // #ifndef POISE_ITERATOR_HPP
