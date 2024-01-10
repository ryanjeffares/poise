#ifndef POISE_TUPE_HPP
#define POISE_TUPE_HPP

#include "Iterable.hpp"
#include "../Object.hpp"

namespace poise::objects::iterables {
class Tuple : public Object, public Iterable
{
public:
    explicit Tuple(std::vector<runtime::Value> data);
    Tuple(runtime::Value key, runtime::Value value);

    ~Tuple() override = default;

    [[nodiscard]] auto begin() noexcept -> IteratorType override;
    [[nodiscard]] auto end() noexcept -> IteratorType override;
    auto incrementIterator(IteratorType& iterator) noexcept -> void override;
    auto isAtEnd(const IteratorType& iterator) noexcept -> bool override;
    [[nodiscard]] auto size() const noexcept -> usize override;
    [[nodiscard]] auto ssize() const noexcept -> isize override;
    auto unpack(std::vector<runtime::Value>& stack) const noexcept -> void override;

    [[nodiscard]] auto asIterable() noexcept -> Iterable* override;
    [[nodiscard]] auto asTuple() noexcept -> Tuple* override;

    [[nodiscard]] auto toString() const noexcept -> std::string override;
    [[nodiscard]] auto type() const noexcept -> runtime::types::Type override;
    [[nodiscard]] auto iterable() const noexcept -> bool override;

    [[nodiscard]] auto at(isize index) const -> const runtime::Value&;

private:
    friend class hashables::Dict;

    [[nodiscard]] auto atMut(isize index) -> runtime::Value&;
};
}   // namespace poise::objects::iterables
 
#endif  // #ifndef POISE_TUPE_HPP 
