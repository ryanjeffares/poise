#ifndef DUAL_INDEX_MAP_HPP
#define DUAL_INDEX_MAP_HPP

#include "../Poise.hpp"

#include <algorithm>
#include <functional>
#include <type_traits>
#include <vector>

namespace poise::utils {
template<typename T>
concept DualIndexableEntry = requires(T value)
{
    { std::hash<T>{}(value) } -> std::convertible_to<usize>;

    std::is_nothrow_default_constructible_v<T>;

    std::is_nothrow_move_constructible_v<T>;
    std::is_nothrow_move_assignable_v<T>;
    std::is_nothrow_copy_constructible_v<T>;
    std::is_nothrow_copy_assignable_v<T>;
};

template<DualIndexableEntry ValueType>
class DualIndexSet
{
public:
    DualIndexSet()
        : m_data{s_initialCapacity}
        , m_size{0_uz}
        , m_capacity{s_initialCapacity}
    {
    }
    
    auto clear() noexcept -> void
    {
        m_data.clear();
        m_data.resize(s_initialCapacity);
        m_size = 0_uz;
        m_capacity = s_initialCapacity;
    }

    auto insert(ValueType value) noexcept -> usize
    {
        const auto hash = hashValue(value);
        auto index = hash % m_capacity;

        auto entry = Entry{
            .value = std::move(value),
            .hash = hash,
            .distance = 0_uz,
            .occupied = true,
        };

        while (m_data[index].occupied) {
            auto& old = m_data[index];
            if (old.hash == hash) {
                return hash;
            }

            if (entry.distance > old.distance) {
                std::swap(entry, old);
            }

            entry.distance++;
            index = (index + 1_uz) % m_capacity;
        }

        m_data[index] = std::move(entry);
        m_size++;
        checkLoad();

        return hash;
    }

    [[nodiscard]] auto remove(const ValueType& value) noexcept -> bool
    {
        return remove(hashValue(value));
    }

    [[nodiscard]] auto remove(usize hash) noexcept -> bool
    {
        auto index = hash % m_capacity;

        while (m_data[index].occupied) {
            auto& entry = m_data[index];

            if (entry.hash == hash) {
                entry = Entry{};

                index++;
                while (m_data[index].occupied && m_data[index].distance > 0_uz) {
                    std::swap(entry, m_data[index]);
                    index = (index + 1_uz) % m_capacity;
                }

                m_size--;
                return true;
            }

            index = (index + 1_uz) % m_capacity;
        }

        return false;
    }

    [[nodiscard]] auto find(usize hash) const noexcept -> const ValueType&
    {
        auto index = hash % m_capacity;

        while (m_data[index].occupied) {
            const auto& entry = m_data[index];
            if (entry.hash == hash) {
                return entry.value;
            }

            index = (index + 1_uz) % m_capacity;
        }

        POISE_UNREACHABLE();
    }

    [[nodiscard]] auto size() const noexcept -> usize
    {
        return m_size;
    }

    [[nodiscard]] auto capacity() const noexcept -> usize
    {
        return m_capacity;
    }

private:
    struct Entry
    {
        ValueType value{};
        usize hash{};
        usize distance{};
        bool occupied{};
    };

    static constexpr auto s_initialCapacity = 8_uz;
    static constexpr auto s_loadFactor = 0.75f;

    static auto hashValue(const ValueType& value) -> usize
    {
        return std::hash<ValueType>{}(value);
    }

    auto checkLoad() noexcept -> void
    {
        if (static_cast<f32>(m_size) / static_cast<f32>(m_capacity) >= s_loadFactor) {
            growAndRehash();
        }
    }

    auto growAndRehash() noexcept -> void
    {
        std::vector<Entry> entries;
        entries.reserve(m_size);

        for (auto& entry : m_data) {
            if (entry.occupied) {
                entry.distance = 0;
                entries.emplace_back(std::move(entry));
            }
        }

        m_capacity *= 2_uz;
        m_data.resize(m_capacity);
        std::ranges::fill(m_data, Entry{});

        // no need to actually recompute hashes, we already know the hashes
        // just redistribute and handle collisions, we know there are no duplicates
        for (auto& entry : entries) {
            auto index = entry.hash % m_capacity;
            while (m_data[index].occupied) {
                auto& old = m_data[index];

                if (entry.distance > old.distance) {
                    std::swap(entry, old);
                }

                entry.distance++;
                index = (index + 1_uz) % m_capacity;
            }

            m_data[index] = std::move(entry);
        }
    }

    std::vector<Entry> m_data;
    usize m_size, m_capacity;
}; // class DualIndexMap
}  // namespace poise::utils

#endif // #ifndef DUAL_INDEX_MAP_HPP

