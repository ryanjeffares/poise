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
    explicit PoiseList(std::vector<runtime::Value> data);
    ~PoiseList() override = default;

    [[nodiscard]] auto begin() noexcept -> IteratorType override;
    [[nodiscard]] auto end() noexcept -> IteratorType override;
    auto incrementIterator(IteratorType& iterator) noexcept -> void override;
    auto isAtEnd(const IteratorType& iterator) noexcept -> bool override;

    [[nodiscard]] auto asList() noexcept -> iterables::PoiseList* override;

    [[nodiscard]] auto toString() const noexcept -> std::string override;
    [[nodiscard]] auto type() const noexcept -> runtime::types::Type override;
    [[nodiscard]] auto iterable() const noexcept -> bool override;

    [[nodiscard]] auto empty() const noexcept -> bool;
    [[nodiscard]] auto size() const noexcept -> usize;

    [[nodiscard]] auto at(usize index) const -> const runtime::Value&;
    [[nodiscard]] auto at(usize index) -> runtime::Value&;

    auto append(runtime::Value value) noexcept -> void;
    [[nodiscard]] auto insert(usize index, runtime::Value value) noexcept -> bool;
    [[nodiscard]] auto remove(const runtime::Value& value) noexcept -> i64;
    [[nodiscard]] auto removeFirst(const runtime::Value& value) noexcept -> bool;
    [[nodiscard]] auto removeAt(usize index) noexcept -> bool;
};
}   // namespace poise::objects::iterables

#endif  // #ifndef POISE_LIST_HPP
