//
// Created by ryand on 13/12/2023.
//

#ifndef POISE_LIST_HPP
#define POISE_LIST_HPP

#include "PoiseIterable.hpp"

namespace poise::objects::iterables {
class PoiseList : public PoiseIterable
{
public:
    explicit PoiseList(std::vector<runtime::Value> data);
    ~PoiseList() override = default;

    [[nodiscard]] auto begin() noexcept -> IteratorType override;
    [[nodiscard]] auto end() noexcept -> IteratorType override;
    auto incrementIterator(IteratorType& iterator) noexcept -> void override;
    auto isAtEnd(const IteratorType& iterator) noexcept -> bool override;

    auto append(runtime::Value value) noexcept -> void;
    [[nodiscard]] auto insert(runtime::Value value, usize index) noexcept -> bool;
    [[nodiscard]] auto remove(const runtime::Value& value) noexcept -> i64;
    [[nodiscard]] auto removeFirst(const runtime::Value& value) noexcept -> bool;
    [[nodiscard]] auto removeAt(usize index) noexcept -> bool;
};
}   // namespace poise::objects::iterables

#endif  // #ifndef POISE_LIST_HPP
