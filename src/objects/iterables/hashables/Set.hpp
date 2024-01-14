#ifndef POISE_SET_HPP
#define POISE_SET_HPP

#include "Hashable.hpp"

namespace poise::objects::iterables::hashables {
class Set : public Object, public Hashable
{
public:
    Set() = default;
    explicit Set(std::span<runtime::Value> data);
    explicit Set(runtime::Value value);

    [[nodiscard]] auto begin() noexcept -> IteratorType override;
    [[nodiscard]] auto end() noexcept -> IteratorType override;
    auto incrementIterator(IteratorType& iterator) noexcept -> void override;
    auto isAtEnd(const IteratorType& iterator) noexcept -> bool override;
    auto unpack(std::vector<runtime::Value>& stack) const noexcept -> void override;

    [[nodiscard]] auto asIterable() noexcept -> Iterable* override;
    [[nodiscard]] auto asHashable() noexcept -> Hashable* override;
    [[nodiscard]] auto asSet() noexcept -> Set* override;

    [[nodiscard]] auto toString() const noexcept -> std::string override;
    [[nodiscard]] auto type() const noexcept -> runtime::types::Type override;
    [[nodiscard]] auto iterable() const -> bool override;

    [[nodiscard]] auto contains(const runtime::Value& value) const noexcept -> bool;
    auto tryInsert(runtime::Value value) noexcept -> bool;

    [[nodiscard]] auto isSubset(const Set& other) const noexcept -> bool;
    [[nodiscard]] auto isSuperset(const Set& other) const noexcept -> bool;

    [[nodiscard]] auto unionWith(const Set& other) const noexcept -> runtime::Value;
    [[nodiscard]] auto intersection(const Set& other) const noexcept -> runtime::Value;
    [[nodiscard]] auto difference(const Set& other) const noexcept -> runtime::Value;
    [[nodiscard]] auto symmetricDifference(const Set& other) const noexcept -> runtime::Value;

protected:
    auto growAndRehash() noexcept -> void override;

private:
    auto addValue(usize index, runtime::Value value) noexcept -> void;
};
} // namespace poise::objects::iterables::hashables

#endif // #ifndef POISE_SET_HPP

