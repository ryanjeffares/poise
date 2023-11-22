#ifndef POISE_HPP
#define POISE_HPP

#include <boost/stacktrace.hpp>
#include <fmt/core.h>
#include <source_location/source_location.hpp>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace poise
{
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
} // namespace grace

#ifndef POISE_ASSERT
#define POISE_ASSERT(condition, message)                                                            \
    do {                                                                                            \
        if (!(condition)) {                                                                         \
            const auto stacktrace = boost::stacktrace::stacktrace{};                                \
            const auto stackTop = stacktrace.begin();                                               \
            fmt::print(stderr, "Assertion failed with expression '{}': {}\n", #condition, message); \
            fmt::print(stderr, "At {}:{}\n", stackTop->source_file(), stackTop->name());            \
            fmt::print(stderr, "Stacktrace:\n{}\n", boost::stacktrace::to_string(stacktrace));      \
            std::exit(-1);                                                                          \
        }                                                                                           \
    } while (false)
#endif

#ifndef POISE_UNREACHABLE
#define POISE_UNREACHABLE() POISE_ASSERT(false, "Unreachable code")
#endif

#endif