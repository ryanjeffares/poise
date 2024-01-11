#ifndef POISE_SET_HPP
#define POISE_SET_HPP

#include "Hashable.hpp"

namespace poise::objects::iterables::hashables {
class Set : public Object, public Hashable
{
public:
    explicit Set(std::vector<runtime::Value> data);

    [[nodiscard]] auto begin() noexcept -> IteratorType override;
    [[nodiscard]] auto end() noexcept -> IteratorType override;
    auto incrementIterator(IteratorType& iterator) noexcept -> void override;
    auto isAtEnd(const IteratorType& iterator) noexcept -> bool override;
    auto unpack(std::vector<runtime::Value>& stack) const noexcept -> void override;

    [[nodiscard]] auto asIterable() noexcept -> iterables::Iterable* override;
    [[nodiscard]] auto asHashable() noexcept -> iterables::hashables::Hashable* override;
    [[nodiscard]] auto asSet() noexcept -> iterables::hashables::Set* override;

    [[nodiscard]] auto toString() const noexcept -> std::string override;
    [[nodiscard]] auto type() const noexcept -> runtime::types::Type override;
    [[nodiscard]] auto iterable() const -> bool override;

    [[nodiscard]] auto contains(const runtime::Value& value) const noexcept -> bool;
    auto tryInsert(runtime::Value value) noexcept -> bool;

protected:
    auto growAndRehash() noexcept -> void override;

private:
    auto addValue(usize index, bool isNewValue, runtime::Value value) noexcept -> void;
};
} // namespace poise::objects::iterables::hashables

#endif // #ifndef POISE_SET_HPP

