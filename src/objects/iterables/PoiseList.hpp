//
// Created by ryand on 13/12/2023.
//

#ifndef POISE_LIST_HPP
#define POISE_LIST_HPP

#include "../PoiseObject.hpp"
#include "PoiseIterable.hpp"

namespace poise::objects::iterables {
class PoiseList : public PoiseObject, public PoiseIterable
{
public:
    explicit PoiseList(runtime::Value value);
    explicit PoiseList(std::vector<runtime::Value> data);
    ~PoiseList() override = default;

    [[nodiscard]] auto begin() noexcept -> IteratorType override;
    [[nodiscard]] auto end() noexcept -> IteratorType override;
    auto incrementIterator(IteratorType& iterator) noexcept -> void override;
    auto isAtEnd(const IteratorType& iterator) noexcept -> bool override;
    [[nodiscard]] auto size() const noexcept -> usize override;
    [[nodiscard]] auto ssize() const noexcept -> isize override;
    auto unpack(std::vector<runtime::Value>& stack) const noexcept -> void override;

    [[nodiscard]] auto asIterable() noexcept -> PoiseIterable* override;
    [[nodiscard]] auto asList() noexcept -> PoiseList* override;

    [[nodiscard]] auto toString() const noexcept -> std::string override;
    [[nodiscard]] auto type() const noexcept -> runtime::types::Type override;
    [[nodiscard]] auto iterable() const noexcept -> bool override;

    [[nodiscard]] auto empty() const noexcept -> bool;

    [[nodiscard]] auto at(isize index) const -> const runtime::Value&;
    [[nodiscard]] auto at(isize index) -> runtime::Value&;

    auto append(runtime::Value value) noexcept -> void;
    [[nodiscard]] auto insert(usize index, runtime::Value value) noexcept -> bool;
    [[nodiscard]] auto remove(const runtime::Value& value) noexcept -> i64;
    [[nodiscard]] auto removeFirst(const runtime::Value& value) noexcept -> bool;
    [[nodiscard]] auto removeAt(usize index) noexcept -> bool;
    auto clear() noexcept -> void;

    [[nodiscard]] auto repeat(isize n) const -> runtime::Value;
    [[nodiscard]] auto concat(const PoiseList& other) const noexcept -> runtime::Value;
};
}   // namespace poise::objects::iterables

#endif  // #ifndef POISE_LIST_HPP
