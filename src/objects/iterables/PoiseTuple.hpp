#ifndef POISE_TUPE_HPP
#define POISE_TUPE_HPP

#include "PoiseIterable.hpp"
#include "../PoiseObject.hpp"

namespace poise::objects::iterables {
class PoiseTuple : public PoiseObject, public PoiseIterable
{
public:
    explicit PoiseTuple(std::vector<runtime::Value> data);
    ~PoiseTuple() override = default;

    [[nodiscard]] auto begin() noexcept -> IteratorType override;
    [[nodiscard]] auto end() noexcept -> IteratorType override;
    auto incrementIterator(IteratorType& iterator) noexcept -> void override;
    auto isAtEnd(const IteratorType& iterator) noexcept -> bool override;
    [[nodiscard]] auto size() const noexcept -> usize override;
    [[nodiscard]] auto ssize() const noexcept -> isize override;
    auto unpack(std::vector<runtime::Value>& stack) const noexcept -> void override;

    [[nodiscard]] auto asIterable() noexcept -> PoiseIterable* override;
    [[nodiscard]] auto asTuple() noexcept -> PoiseTuple* override;

    [[nodiscard]] auto toString() const noexcept -> std::string override;
    [[nodiscard]] auto type() const noexcept -> runtime::types::Type override;
    [[nodiscard]] auto iterable() const noexcept -> bool override;

    [[nodiscard]] auto at(isize index) const -> const runtime::Value&;
};
}   // namespace poise::objects::iterables
 
#endif  // #ifndef POISE_TUPE_HPP 
