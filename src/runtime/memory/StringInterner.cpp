#include "StringInterner.hpp"

#include <fmt/core.h>

#include <functional>
#include <ranges>
#include <vector>

namespace poise::runtime::memory {
[[nodiscard]] static auto hashString(const std::string& string) noexcept -> usize
{
    return std::hash<std::string>{}(string);
}

class StringPool
{
public:
    StringPool(const StringPool&) = delete;
    StringPool& operator=(const StringPool&) = delete;

    static auto instance() noexcept -> StringPool&
    {
        static auto s = StringPool{};
        return s;
    }

    auto initialise() noexcept -> void
    {
        m_size = 0_uz;
        m_capacity = s_initialCapacity;
        m_data.clear();
        m_data.resize(m_capacity);
    }

    auto insert(std::string string) noexcept -> usize
    {
        const auto hash = hashString(string);
        auto index = hash % m_capacity;
        
        auto newEntry = InternedString{
            .string = std::move(string),
            .hash = hash,
            .distance = 0_uz,
            .occupied = true,
        };

        while (m_data[index].occupied) {
            auto& oldEntry = m_data[index];
            
            if (oldEntry.hash == hash) {
                return hash;
            }

            if (newEntry.distance > oldEntry.distance) {
                std::swap(newEntry, oldEntry);
            }

            newEntry.distance++;
            index = (index + 1_uz) % m_capacity;
        }

        m_data[index] = std::move(newEntry);
        m_size++;

        if (static_cast<f32>(m_size) / static_cast<f32>(m_capacity) >= s_loadFactor) {
            growAndRehash();
        }

        return hash;
    }

    [[nodiscard]] auto remove(const std::string& string) noexcept -> bool
    {
        return remove(hashString(string));
    }

    [[nodiscard]] auto remove(usize hash) noexcept -> bool
    {
        auto index = hash % m_capacity;

        while (m_data[index].occupied) {
            auto& entry = m_data[index];

            if (entry.hash == hash) {
                entry = InternedString{
                    .string = {},
                    .hash = 0_uz,
                    .distance = 0_uz,
                    .occupied = false,
                };

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

    [[nodiscard]] auto find(usize hash) const noexcept -> const std::string&
    {
        auto index = hash % m_capacity;

        while (m_data[index].occupied) {
            const auto& entry = m_data[index];

            if (entry.hash == hash) {
                return entry.string;
            }

            index = (index + 1_uz) % m_capacity;
        }
        
        POISE_UNREACHABLE();
    }

    [[nodiscard]] auto count() const noexcept -> usize
    {
        return m_size;
    }

    auto dumpStrings() const noexcept -> void
    {
        fmt::print("Interned Strings:\n");
        for (const auto& entry : m_data) {
            if (entry.occupied) {
                fmt::print("\t{}\n", entry.string);
            }
        }
    }

private:
    StringPool()
        : m_data{s_initialCapacity}
        , m_size{0_uz}
        , m_capacity{s_initialCapacity}
    {

    }

    auto growAndRehash() noexcept -> void
    {
        std::vector<std::string> entries;
        entries.reserve(m_size);

        for (auto& entry : m_data) {
            if (entry.occupied) {
                entries.emplace_back(std::move(entry.string));
            }
        }

        m_size = 0_uz;
        m_capacity *= 2_uz;

        m_data.resize(m_capacity);
        std::ranges::fill(m_data, InternedString{});

        for (auto& string : entries) {
            insert(std::move(string));
        }
    }

    struct InternedString
    {
        std::string string{};
        usize hash{};
        usize distance{};
        bool occupied{};
    };

    static constexpr auto s_initialCapacity = 8_uz;
    static constexpr auto s_loadFactor = 0.75f;

    std::vector<InternedString> m_data;
    usize m_size, m_capacity;
};

auto intialiseStringInterning() noexcept -> void
{
    StringPool::instance().initialise();
}

auto internString(std::string string) noexcept -> usize
{
    return StringPool::instance().insert(std::move(string));
}

auto removeInternedString(const std::string& string) noexcept -> bool
{
    return StringPool::instance().remove(string);
}

auto removeInternedStringId(usize hash) noexcept -> bool
{
    return StringPool::instance().remove(hash);
}

auto internedStringCount() noexcept -> usize
{
    return StringPool::instance().count();
}

auto findInternedString(usize hash) noexcept -> const std::string&
{
    return StringPool::instance().find(hash);
}

auto dumpStrings() noexcept -> void
{
    StringPool::instance().dumpStrings();
}
} // namespace poise::runtime::memory

