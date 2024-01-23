//
// Created by ryand on 13/12/2023.
//

#ifndef POISE_ITERABLE_HPP
#define POISE_ITERABLE_HPP

#include "../../Poise.hpp"
#include "Iterator.hpp"
#include "../Object.hpp"
#include "../../runtime/Value.hpp"

#include <span>

namespace poise::objects::iterables {
class Iterable : public Object
{
public:
    using IteratorType = Iterator::IteratorType;
    using DifferenceType = std::vector<runtime::Value>::difference_type;

    Iterable() = default;
    explicit Iterable(usize initialSize, const runtime::Value& defaultValue = runtime::Value::none());
    explicit Iterable(std::vector<runtime::Value> data);
     ~Iterable() override;

    [[nodiscard]] auto asIterable() noexcept -> Iterable* override;
    auto findObjectMembers(std::vector<Object*>& objects) const noexcept -> void override;
    auto removeObjectMembers() noexcept -> void override;
    [[nodiscard]] auto anyMemberMatchesRecursive(const Object* object) const noexcept -> bool override;

    [[nodiscard]] virtual auto begin() noexcept -> IteratorType = 0;
    [[nodiscard]] virtual auto end() noexcept -> IteratorType = 0;
    virtual auto incrementIterator(IteratorType& iterator) noexcept -> void = 0;
    virtual auto isAtEnd(const IteratorType& iterator) noexcept -> bool = 0;
    [[nodiscard]] virtual auto size() const noexcept -> usize = 0;
    [[nodiscard]] virtual auto ssize() const noexcept -> isize = 0;
    virtual auto unpack(std::vector<runtime::Value>& stack) const noexcept -> void = 0;

    auto addIterator(Iterator* iterator) noexcept -> void;
    auto removeIterator(Iterator* iterator) noexcept -> void;
    [[nodiscard]] auto data() const noexcept -> std::span<const runtime::Value>;

protected:
    auto invalidateIterators() const noexcept -> void;

    std::vector<runtime::Value> m_data;
    std::vector<Iterator*> m_activeIterators;
};
}   // namespace poise::objects::iterables

#endif  // #ifndef POISE_ITERABLE_HPP
