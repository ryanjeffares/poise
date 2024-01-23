#ifndef POISE_HASHABLE_HPP
#define POISE_HASHABLE_HPP

#include "../Iterable.hpp"

namespace poise::objects::iterables::hashables {
class Hashable : public Iterable
{
public:
    Hashable();
    explicit Hashable(usize initialCapacity, const runtime::Value& defaultValue = runtime::Value::none());

    [[nodiscard]] auto asHashable() noexcept -> Hashable* override;

    [[nodiscard]] auto size() const noexcept -> usize override;
    [[nodiscard]] auto ssize() const noexcept -> isize override;
    [[nodiscard]] auto capacity() const noexcept -> usize;
    [[nodiscard]] auto toVector() const noexcept -> std::vector<runtime::Value>;

    static constexpr auto s_initialCapacity = 8_uz;
    static constexpr auto s_growFactor = 2_uz;
    static constexpr auto s_threshold = 0.75f;

protected:
    virtual auto growAndRehash() noexcept -> void = 0;
    
    enum class CellState
    {
        NeverUsed,
        Occupied,
        Tombstone,
    };

    usize m_size{};
    usize m_capacity = s_initialCapacity;

    std::vector<CellState> m_cellStates;
};
} // namespace poise::objects::iterables::hashables

#endif  // #ifndef POISE_HASHABLE_HPP

