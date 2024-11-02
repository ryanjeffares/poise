#ifndef POISE_DUAL_INDEX_SET_HPP
#define POISE_DUAL_INDEX_SET_HPP

#include "../Poise.hpp"

#include <boost/unordered/unordered_flat_map.hpp>

namespace poise::utils {
template<typename Key, typename KeyHash = boost::hash<Key>>
class DualIndexSet
{
public:
    using Hasher = KeyHash;
    using Map = boost::unordered_flat_map<usize, Key>;
    using Iterator = typename Map::iterator;
    using ConstIterator = typename Map::const_iterator;

    auto begin() noexcept -> Iterator
    {
        return m_map.begin();
    }

    auto begin() const noexcept -> ConstIterator
    {
        return m_map.begin();
    }

    auto end() noexcept -> Iterator
    {
        return m_map.end();
    }

    auto end() const noexcept -> ConstIterator
    {
        return m_map.end();
    }

    auto cbegin() const noexcept -> ConstIterator
    {
        return m_map.cbegin();
    }

    auto cend() const noexcept -> ConstIterator
    {
        return m_map.cend();
    }

    [[nodiscard]] auto empty() const noexcept -> bool
    {
        return m_map.empty();
    }

    [[nodiscard]] auto size() const noexcept -> usize
    {
        return m_map.size();
    }

    auto insert(Key key) -> std::tuple<usize, Iterator, bool>
    {
        const auto hash = m_keyHash(key);
        const auto [it, inserted] = m_map.emplace(hash, std::move(key));
        return {hash, it, inserted};
    }

    auto find(usize hash) -> Iterator
    {
        return m_map.find(hash);
    }

    auto find(usize hash) const -> ConstIterator
    {
        return m_map.find(hash);
    }

    [[nodiscard]] auto contains(usize hash) const -> bool
    {
        return m_map.contains(hash);
    }

    auto remove(usize hash) -> bool
    {
        return m_map.erase(hash) > 0_uz;
    }

    auto clear() noexcept -> void
    {
        m_map.clear();
    }

private:
    Map m_map;
    Hasher m_keyHash;
}; // class DualIndexSet
} // namespace poise::utils

#endif // #ifndef POISE_DUAL_INDEX_SET_HPP

