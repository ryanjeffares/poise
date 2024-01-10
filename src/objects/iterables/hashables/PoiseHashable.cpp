#include "PoiseHashable.hpp"

namespace poise::objects::iterables::hashables {
PoiseHashable::PoiseHashable()
    : PoiseIterable{s_initialCapacity, runtime::Value::none()}
    , m_cellStates(s_initialCapacity, CellState::NeverUsed)
{

}

PoiseHashable::PoiseHashable(usize initialCapacity, const runtime::Value& defaultValue)
    : PoiseIterable{initialCapacity, defaultValue}
    , m_cellStates(initialCapacity, CellState::NeverUsed)
{

}

auto PoiseHashable::toVector() const noexcept -> std::vector<runtime::Value>
{
    std::vector<runtime::Value> res;
    res.reserve(size());

    for (auto i = 0_uz; i < m_capacity; i++) {
        if (m_cellStates[i] == CellState::Occupied) {
            res.push_back(m_data[i]);
        }
    }

    return res;
}
} // namespace poise::objects::iterables::hashables
