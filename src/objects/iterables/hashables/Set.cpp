#include "Set.hpp"
#include "../Range.hpp"

#include <ranges>

namespace poise::objects::iterables::hashables {
Set::Set(std::span<runtime::Value> data)
{
    for (auto& value : data) {
        tryInsert(std::move(value));
    }
}

Set::Set(runtime::Value value)
{
    switch (value.type()) {
        case runtime::types::Type::Dict:
        case runtime::types::Type::Set: {
            const auto hashable = value.object()->asHashable();
            for (auto values = hashable->toVector(); auto& v : values) {
                tryInsert(std::move(v));
            }

            break;
        }
        case runtime::types::Type::Range: {
            const auto range = value.object()->asRange();
            for (const auto values = range->toVector(); const auto& v : values) {
                tryInsert(v);
            }

            break;
        }
        case runtime::types::Type::List:
        case runtime::types::Type::Tuple: {
            const auto iterable = value.object()->asIterable();
            for (const auto values = iterable->data(); const auto& v : values) {
                tryInsert(v);
            }

            break;
        }
        case runtime::types::Type::String: {
            for (const auto& string = value.string(); const auto c : string) {
                tryInsert(std::string(1_uz, c));
            }

            break;
        }
        default: {
            tryInsert(std::move(value));
            break;
        }
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
    usize index;
    do {
        ++iterator;
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


auto Set::asSet() noexcept -> Set*
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

        if (m_data[i].object() && m_data[i].object()->anyMemberMatchesRecursive(this)) {
            res.append("...");
        } else {
            res.append(m_data[i].toString());
        }

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
                addValue(index, std::move(value));
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
        }
    }
}

auto Set::remove(const runtime::Value& value) noexcept -> bool
{
    auto index = value.hash() % capacity();

    while (true) {
        switch (m_cellStates[index]) {
            case CellState::NeverUsed: {
                return false;
            }
            case CellState::Occupied: {
                if (m_data[index] == value) {
                    m_data[index] = runtime::Value::none();
                    m_cellStates[index] = CellState::Tombstone;
                    m_size--;
                    invalidateIterators();
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
        }
    }
}

auto Set::isSubset(const Set& other) const noexcept -> bool
{
    if (size() == 0_uz || this == &other) {
        return true;
    }

    for (auto i = 0_uz; i < capacity(); i++) {
        if (m_cellStates[i] == CellState::Occupied && !other.contains(m_data[i])) {
            return false;
        }
    }

    return true;
}

auto Set::isSuperset(const Set& other) const noexcept -> bool
{
    if (this == &other) {
        return true;
    }

    for (auto i = 0_uz; i < other.capacity(); i++) {
        if (other.m_cellStates[i] == CellState::Occupied && !contains(other.m_data[i])) {
            return false;
        }
    }

    return true;
}

auto Set::unionWith(const Set& other) const noexcept -> runtime::Value
{
    auto value = runtime::Value::createObject<Set>();
    const auto newSet = value.object()->asSet();

    for (auto i = 0_uz; i < capacity(); i++) {
        if (m_cellStates[i] == CellState::Occupied) {
            newSet->tryInsert(m_data[i]);
        }
    }

    for (auto i = 0_uz; i < other.capacity(); i++) {
        if (other.m_cellStates[i] == CellState::Occupied) {
            newSet->tryInsert(other.m_data[i]);
        }
    }

    return value;
} 

auto Set::intersection(const Set& other) const noexcept -> runtime::Value
{
    auto value = runtime::Value::createObject<Set>();
    const auto newSet = value.object()->asSet();

    for (auto i = 0_uz; i < capacity(); i++) {
        if (m_cellStates[i] == CellState::Occupied && other.contains(m_data[i])) {
            newSet->tryInsert(m_data[i]);
        }
    }

    for (auto i = 0_uz; i < other.capacity(); i++) {
        if (other.m_cellStates[i] == CellState::Occupied && contains(other.m_data[i])) {
            newSet->tryInsert(other.m_data[i]);
        }
    }

    return value;
} 

auto Set::difference(const Set& other) const noexcept -> runtime::Value
{
    auto value = runtime::Value::createObject<Set>();
    const auto newSet = value.object()->asSet();

    for (auto i = 0_uz; i < capacity(); i++) {
        if (m_cellStates[i] == CellState::Occupied && !other.contains(m_data[i])) {
            newSet->tryInsert(m_data[i]);
        }
    }

    return value;     
} 

auto Set::symmetricDifference(const Set& other) const noexcept -> runtime::Value
{
    auto value = runtime::Value::createObject<Set>();
    const auto newSet = value.object()->asSet();

    for (auto i = 0_uz; i < capacity(); i++) {
        if (m_cellStates[i] == CellState::Occupied && !other.contains(m_data[i])) {
            newSet->tryInsert(m_data[i]);
        }
    }

    for (auto i = 0_uz; i < other.capacity(); i++) {
        if (other.m_cellStates[i] == CellState::Occupied && !contains(other.m_data[i])) {
            newSet->tryInsert(other.m_data[i]);
        }
    }

    return value;     
} 

auto Set::growAndRehash() noexcept -> void
{
    auto values = toVector();

    m_capacity *= 2_uz;
    m_size = 0_uz;
    m_data.resize(m_capacity);
    m_cellStates.resize(m_capacity);

    std::ranges::fill(m_data, runtime::Value::none());
    std::ranges::fill(m_cellStates, CellState::NeverUsed);

    for (auto& value : values) {
        tryInsert(std::move(value));
    }
}

auto Set::addValue(usize index, runtime::Value value) noexcept -> void
{
    m_size++;
    m_data[index] = std::move(value);
    m_cellStates[index] = CellState::Occupied;
    invalidateIterators();

    if (static_cast<f32>(size()) / static_cast<f32>(capacity()) >= s_threshold) {
        growAndRehash();
    }
}
} // namespace poise::objects::iterables::hashables

