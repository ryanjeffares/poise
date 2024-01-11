#include "Hashable.hpp"

namespace poise::objects::iterables::hashables {
Hashable::Hashable()
    : Iterable{s_initialCapacity, runtime::Value::none()}
    , m_cellStates(s_initialCapacity, CellState::NeverUsed)
{

}

Hashable::Hashable(usize initialCapacity, const runtime::Value& defaultValue)
    : Iterable{initialCapacity, defaultValue}
    , m_cellStates(initialCapacity, CellState::NeverUsed)
{

}

auto Hashable::size() const noexcept -> usize
{
    return m_size;
}

auto Hashable::ssize() const noexcept -> isize
{
    return static_cast<isize>(m_size);
}

auto Hashable::capacity() const noexcept -> usize
{
    return m_capacity;
}

auto Hashable::toVector() const noexcept -> std::vector<runtime::Value>
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

