#ifndef POISE_HPP
#define POISE_HPP

#include <boost/stacktrace.hpp>

#include <fmt/core.h>

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <optional>
#include <string>

namespace poise {
using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using usize = std::size_t;
using isize = std::ptrdiff_t;

using f32 = float;
using f64 = double;

inline constexpr auto operator ""_i8(unsigned long long int value) noexcept -> i8
{
    return static_cast<i8>(value);
}

inline constexpr auto operator ""_i16(unsigned long long int value) noexcept -> i16
{
    return static_cast<i16>(value);
}

inline constexpr auto operator ""_i32(unsigned long long int value) noexcept -> i32
{
    return static_cast<i32>(value);
}

inline constexpr auto operator ""_i64(unsigned long long int value) noexcept -> i64
{
    return static_cast<i64>(value);
}

inline constexpr auto operator ""_u8(unsigned long long int value) noexcept -> u8
{
    return static_cast<u8>(value);
}

inline constexpr auto operator ""_u16(unsigned long long int value) noexcept -> u16
{
    return static_cast<u16>(value);
}

inline constexpr auto operator ""_u32(unsigned long long int value) noexcept -> u32
{
    return static_cast<u32>(value);
}

inline constexpr auto operator ""_u64(unsigned long long int value) noexcept -> u64
{
    return static_cast<u64>(value);
}

inline constexpr auto operator ""_uz(unsigned long long int value) noexcept -> usize
{
    return static_cast<usize>(value);
}

inline constexpr auto operator ""_iz(unsigned long long int value) noexcept -> isize
{
    return static_cast<isize>(value);
}

inline constexpr auto operator ""_f32(long double value) noexcept -> f32
{
    return static_cast<f32>(value);
}

inline constexpr auto operator ""_f64(long double value) noexcept -> f64
{
    return static_cast<f64>(value);
}

#if defined(POISE_MSVC) && !defined(CRT_SECURE_NO_WARNINGS)
inline auto getEnv(const char* varName) noexcept -> std::string
{
    std::string res;
    char* pValue{};
    auto len = 0_uz;
    auto err = _dupenv_s(&pValue, &len, varName);
    if (err) {
        free(pValue);
    } else {
        res = pValue;
    }
    return res;
}
#else
inline auto getEnv(const char* varName) noexcept -> std::string
{
    return std::getenv(varName);
}
#endif

inline auto getStdPath() noexcept -> std::optional<std::filesystem::path>
{
    static std::optional<std::filesystem::path> result;
    static bool found = false;

    if (!found) {
        found = true;
        const auto var = getEnv("POISE_STD_PATH");
        if (!var.empty()) {
            result = std::filesystem::path{var};
        } else {
            return std::nullopt;
        }
    }

    return result;
}
} // namespace grace

#ifndef POISE_ASSERT
#ifdef POISE_DEBUG
#define POISE_ASSERT(condition, message)                                                            \
    do {                                                                                            \
        if (!(condition)) {                                                                         \
            const auto stacktrace = boost::stacktrace::stacktrace{};                                \
            const auto stackTop = stacktrace.cbegin();                                              \
            fmt::print(stderr, "Assertion failed with expression '{}': {}\n", #condition, message); \
            fmt::print(stderr, "At {}:{}\n", stackTop->source_file(), stackTop->name());            \
            fmt::print(stderr, "Stacktrace:\n{}\n", boost::stacktrace::to_string(stacktrace));      \
            std::exit(-1);                                                                          \
        }                                                                                           \
    } while (false)
#else
#define POISE_ASSERT(condition, message)
#endif
#endif

#ifndef POISE_UNREACHABLE
#define POISE_UNREACHABLE() POISE_ASSERT(false, "Unreachable code")
#endif

#ifdef POISE_MSVC
#undef min
#undef max
#endif

#endif  // #ifndef POISE_HPP
