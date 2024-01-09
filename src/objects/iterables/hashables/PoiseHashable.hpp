#ifndef POISE_HASHABLE_HPP
#define POISE_HASHABLE_HPP

#include "../PoiseIterable.hpp"

namespace poise::objects::iterables::hashables {
class PoiseHashable : public PoiseIterable
{
public:
    PoiseHashable();
    PoiseHashable(usize initialCapacity, const runtime::Value& defaultValue = runtime::Value::none());

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

