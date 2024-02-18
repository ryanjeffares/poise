#include "StringInterner.hpp"
#include "../../utils/DualIndexSet.hpp"

#include <fmt/core.h>

namespace poise::runtime::memory {
static utils::DualIndexSet<std::string> s_stringPool;

auto intialiseStringInterning() noexcept -> void
{
    s_stringPool.clear();
}

auto internString(std::string string) noexcept -> usize
{
    return s_stringPool.insert(std::move(string));
}

auto removeInternedString(const std::string& string) noexcept -> bool
{
    return s_stringPool.remove(string);
}

auto removeInternedStringId(usize hash) noexcept -> bool
{
    return s_stringPool.remove(hash);
}

auto internedStringCount() noexcept -> usize
{
    return s_stringPool.size();
}

auto findInternedString(usize hash) noexcept -> const std::string&
{
    return s_stringPool.find(hash);
}

auto dumpStrings() noexcept -> void
{
    s_stringPool.dump();
}
} // namespace poise::runtime::memory

