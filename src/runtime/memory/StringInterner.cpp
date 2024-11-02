#include "../../Poise.hpp"
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
#ifdef POISE_DEBUG
    fmt::print("Interning {}\n", string);
#endif
    return std::get<0>(s_stringPool.insert(std::move(string)));
}

auto removeInternedString(usize hash) noexcept -> bool
{
    return s_stringPool.remove(hash);
}

auto internedStringCount() noexcept -> usize
{
    return s_stringPool.size();
}

auto findInternedString(usize hash) noexcept -> const std::string&
{
    POISE_ASSERT(s_stringPool.contains(hash), "Tried to find a string that had not been interned");
    return s_stringPool.find(hash)->second;
}
} // namespace poise::runtime::memory

