#ifndef POISE_DICTIONARY_HPP
#define POISE_DICTIONARY_HPP

#include "Hashable.hpp"

#include <span>

namespace poise::objects::iterables::hashables {
class Dict : public Hashable
{
public:
    explicit Dict(std::span<runtime::Value> pairs);
    ~Dict() override = default;

    [[nodiscard]] auto begin() noexcept -> IteratorType override;
    [[nodiscard]] auto end() noexcept -> IteratorType override;
    auto incrementIterator(IteratorType& iterator) noexcept -> void override;
    auto isAtEnd(const IteratorType& iterator) noexcept -> bool override;
    auto unpack(std::vector<runtime::Value>& stack) const noexcept -> void override;

    [[nodiscard]] auto asDictionary() noexcept -> Dict* override;

    [[nodiscard]] auto toString() const noexcept -> std::string override;
    [[nodiscard]] auto type() const noexcept -> runtime::types::Type override;
    [[nodiscard]] auto iterable() const -> bool override;

    [[nodiscard]] auto containsKey(const runtime::Value& key) const noexcept -> bool;
    [[nodiscard]] auto at(const runtime::Value& key) const -> const runtime::Value&;

    [[nodiscard]] auto tryInsert(runtime::Value key, runtime::Value value) noexcept -> bool;
    auto insertOrUpdate(runtime::Value key, runtime::Value value) noexcept -> void;
    [[nodiscard]] auto remove(const runtime::Value& key) noexcept -> bool;
    
protected:
    auto growAndRehash() noexcept -> void override;

private:
    auto addPair(usize index, bool isNewKey, runtime::Value key, runtime::Value value) noexcept -> void;
};
} // namespace poise::objects::iterables::hashables 

#endif // #ifndef POISE_DICTIONARY_HPP

