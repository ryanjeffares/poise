#include "Set.hpp"

namespace poise::objects::iterables::hashables {
Set::Set(std::vector<runtime::Value> data)
{
    for (auto& value : data) {
        tryInsert(std::move(value));
    }
}

auto Set::begin() noexcept -> IteratorType
{
    for (auto i = 0_uz; i < size(); i++) {
        if (m_cellStates[i] == CellState::Occupied) {
            return m_data.begin() + static_cast<isize>(i);
        }
    }

    return end();
}

auto Set::end() noexcept -> IteratorType
{
    return m_data.end();
}

auto Set::incrementIterator(IteratorType& iterator) noexcept -> void
{
    usize index{};
    do {
        iterator++;
        index = static_cast<usize>(std::distance(m_data.begin(), iterator));
    } while (!isAtEnd(iterator) && m_cellStates[index] != CellState::Occupied);
}

auto Set::isAtEnd(const IteratorType& iterator) noexcept -> bool
{
    return iterator == end();
}

auto Set::unpack(std::vector<runtime::Value>& stack) const noexcept -> void
{
    for (auto i = 0_uz; i < capacity(); i++) {
        if (m_cellStates[i] == CellState::Occupied) {
            stack.push_back(m_data[i]);
        }
    }

    stack.emplace_back(size());
}


auto Set::asIterable() noexcept -> iterables::Iterable*
{
    return this;
}

auto Set::asHashable() noexcept -> iterables::hashables::Hashable*
{
    return this;
}

auto Set::asSet() noexcept -> iterables::hashables::Set*
{
    return this;
}


auto Set::toString() const noexcept -> std::string
{
    std::string res = "{";
    usize count = 0_uz;
    for (auto i = 0_uz; i < capacity(); i++) {
        if (m_cellStates[i] != CellState::Occupied) {
            continue;
        }

        res.append(m_data[i].toString());

        if (count++ < size() - 1_uz) {
            res.append(", ");
        }
    }

    res.push_back('}');
    return res;
}

auto Set::type() const noexcept -> runtime::types::Type
{
    return runtime::types::Type::Set;
}

auto Set::iterable() const -> bool
{
    return true;
}

[[nodiscard]] auto Set::contains(const runtime::Value& value) const noexcept -> bool
{
    auto index = value.hash() % capacity();

    while (true) {
        switch (m_cellStates[index]) {
            case CellState::NeverUsed: {
                return false;
            }
            case CellState::Occupied: {
                if (m_data[index] == value) {
                    return true;
                }

                [[fallthrough]];
            }
            case CellState::Tombstone: {
                index = (index + 1_uz) % capacity();
                break;
            }
            default:
                POISE_UNREACHABLE();
                break;
        }
    }
}

auto Set::tryInsert(runtime::Value value) noexcept -> bool
{
    auto index = value.hash() % capacity();

    while (true) {
        switch (m_cellStates[index]) {
            case CellState::Tombstone:
            case CellState::NeverUsed: {
                addValue(index, true, std::move(value));
                return true;
            }
            case CellState::Occupied: {
                if (m_data[index] == value) {
                    return false;
                }

                index = (index + 1_uz) % capacity();
                break;
            }
            default:
                POISE_UNREACHABLE();
                return false;
        }
    }
}

auto Set::growAndRehash() noexcept -> void
{
    auto values = toVector();
    m_capacity *= 2_uz;
    m_size = 0_uz;
    m_data.resize(m_capacity);
    m_cellStates.resize(m_capacity);
    std::fill(m_data.begin(), m_data.end(), runtime::Value::none());
    std::fill(m_cellStates.begin(), m_cellStates.end(), CellState::NeverUsed);

    for (auto& value : values) {
        tryInsert(std::move(value));
    }
}

auto Set::addValue(usize index, bool isNewValue, runtime::Value value) noexcept -> void
{
    if (isNewValue && static_cast<f32>(size()) / static_cast<f32>(capacity()) >= s_threshold) {
        growAndRehash();
    }

    m_size++;
    m_data[index] = std::move(value);
    m_cellStates[index] = CellState::Occupied;
    invalidateIterators();
}
} // namespace poise::objects::iterables::hashables
