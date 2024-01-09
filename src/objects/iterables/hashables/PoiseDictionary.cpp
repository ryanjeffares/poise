#include "PoiseDictionary.hpp"
#include "../PoiseTuple.hpp"
#include "../../PoiseException.hpp"

namespace poise::objects::iterables::hashables {
PoiseDictionary::PoiseDictionary(std::span<runtime::Value> pairs)
{
    // TODO: insert these pairs
}

auto PoiseDictionary::begin() noexcept -> IteratorType
{
    for (auto i = 0_uz; i < m_capacity; i++) {
        if (m_cellStates[i] == CellState::Occupied) {
            return m_data.begin() + i;
        }
    }

    return end();
}

auto PoiseDictionary::end() noexcept -> IteratorType
{
    return m_data.end();
}

auto PoiseDictionary::incrementIterator(IteratorType& iterator) noexcept -> void
{
    usize index{};
    do {
        iterator++;
        index = std::distance(m_data.begin(), iterator);
    } while (!isAtEnd(iterator) && m_cellStates[index] != CellState::Occupied);
}

auto PoiseDictionary::isAtEnd(const IteratorType& iterator) noexcept -> bool
{
    return iterator == end();
}

auto PoiseDictionary::size() const noexcept -> usize
{
    return m_size;
}

auto PoiseDictionary::ssize() const noexcept -> isize
{
    return static_cast<isize>(m_size);
}

auto PoiseDictionary::unpack(std::vector<runtime::Value>& stack) const noexcept -> void
{
    for (auto i = 0_uz; i < m_capacity; i++) {
        if (m_cellStates[i] == CellState::Occupied) {
            stack.push_back(m_data[i]);
        }
    }
}

auto PoiseDictionary::toString() const noexcept -> std::string
{
    std::string res = "{";
    usize count = 0_uz;
    for (auto i = 0_uz; i < m_capacity; i++) {
        if (m_cellStates[i] != CellState::Occupied) {
            continue;
        }

        res.append(m_data[i].toString());
        count++;

        if (count < m_size - 1_uz) {
            res.append(", ");
        }
    }

    return res;
}

auto PoiseDictionary::type() const noexcept -> runtime::types::Type
{
    return runtime::types::Type::Dictionary;
}

auto PoiseDictionary::iterable() const -> bool
{
    return true;
}

auto PoiseDictionary::containsKey(const runtime::Value& key) const noexcept -> bool
{
    const auto hash = key.hash();
    auto index = hash % m_capacity;

    while (true) {
        switch (m_cellStates[index]) {
            case CellState::NeverUsed:
                return false;
            case CellState::Occupied: {
                const auto tuple = m_data[index].object()->asTuple();
                if (tuple->at(0_uz)== key) {
                    return true;
                }

                [[fallthrough]];
            }
            case CellState::Tombstone: {
                index = (index + 1_uz) % m_capacity;
                break;
            }
            default: {
                POISE_UNREACHABLE();
                return false;
            }
        }
    }
}

auto PoiseDictionary::at(const runtime::Value& key) const -> const runtime::Value&
{
    const auto hash = key.hash();
    auto index = hash % m_capacity;

    while (true) {
        switch (m_cellStates[index]) {
            case CellState::NeverUsed:
                throw PoiseException{
                    PoiseException::ExceptionType::KeyNotFound,
                    fmt::format("{} was not present in the Dictionary", key)
                };
            case CellState::Occupied: {
                const auto tuple = m_data[index].object()->asTuple();
                if (tuple->at(0_uz)== key) {
                    return tuple->at(1_uz);
                }

                [[fallthrough]];
            }
            case CellState::Tombstone: {
                index = (index + 1_uz) % m_capacity;
                break;
            }
#ifdef POISE_DEBUG
            default: {
                POISE_UNREACHABLE();
            }
#endif
        }
    }
}

auto PoiseDictionary::tryInsert(runtime::Value key, runtime::Value value) noexcept -> bool
{
    if (containsKey(key)) {
        return false;
    }

    const auto hash = key.hash();
    auto index = hash % m_capacity;

    while (true) {
        switch (m_cellStates[index]) {
            case CellState::NeverUsed:
            case CellState::Tombstone: {
                m_data[index] = runtime::Value::createObject<PoiseTuple>(std::move(key), std::move(value));
                m_cellStates[index] = CellState::Occupied;
                m_size++;
                invalidateIterators();

                if (static_cast<f32>(m_size) / static_cast<f32>(m_capacity) >= s_threshold) {
                    growAndRehash();
                }

                return true;
            }
            case CellState::Occupied: {
                // we know they can't be equal since we already checked if it contains the key
                // so just look ahead
                index = (index + 1_uz) % m_capacity;
                break;
            }
        }
    }
}

auto PoiseDictionary::insertOrUpdate(runtime::Value key, runtime::Value value) noexcept -> void
{
    const auto hash = key.hash();
    auto index = hash % m_capacity;

    while (true) {
        switch (m_cellStates[index]) {
            case CellState::NeverUsed:
            case CellState::Tombstone: {
                addPair(index, std::move(key), std::move(value));
                return;
            }
            case CellState::Occupied: {
                if (m_data[index].object()->asTuple()->at(0_uz) == key) {
                    addPair(index, std::move(key), std::move(value));
                    return;
                }

                index = (index + 1_uz) % m_capacity;
                break;
            }
        }
    }
}

auto PoiseDictionary::addPair(usize index, runtime::Value key, runtime::Value value) noexcept -> void
{
    m_data[index] = runtime::Value::createObject<PoiseTuple>(std::move(key), std::move(value));
    m_cellStates[index] = CellState::Occupied;
    m_size++;
    invalidateIterators();

    if (static_cast<f32>(m_size) / static_cast<f32>(m_capacity) >= s_threshold) {
        growAndRehash();
    }
}
} // namespace poise::objects::iterables::hashables

