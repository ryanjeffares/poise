#ifndef STRING_INTERNER_HPP
#define STRING_INTERNER_HPP

#include "../../Poise.hpp"

#include <string>

namespace poise::runtime::memory {
[[nodiscard]] auto internString(std::string string) noexcept -> usize;
[[nodiscard]] auto removeInternedString(const std::string& string) noexcept -> bool;
[[nodiscard]] auto removeInternedStringId(usize hash) noexcept -> bool;

[[nodiscard]] auto internedStringCount() noexcept -> usize;
[[nodiscard]] auto findInternedString(usize hash) noexcept -> const std::string&;

auto dumpStrings() noexcept -> void;
} // namespace poise::runtime::memory

#endif // #ifndef STRING_INTERNER_HPP

