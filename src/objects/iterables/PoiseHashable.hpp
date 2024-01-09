#ifndef POISE_HASHABLE_HPP
#define POISE_HASHABLE_HPP

#include "PoiseIterable.hpp"

namespace poise::objects::iterables {
class PoiseHashable : public PoiseIterable
{
public:
    PoiseHashable(usize initialSize, const runtime::Value& defaultValue = runtime::Value::none());

    [[nodiscard]] auto toVector() const noexcept -> std::vector<runtime::Value>;

protected:
    static constexpr auto s_initialCapacity = 8_uz;
    static constexpr auto s_growFactor = 2_uz;
    static constexpr auto s_threshold = 0.75f;

    virtual auto growAndRehash() -> void = 0;
    
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
} // namespace poise::objects::iterables

#endif  // #ifndef POISE_HASHABLE_HPP

