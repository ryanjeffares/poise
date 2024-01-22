#include "../src/runtime/memory/StringInterner.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

namespace poise::tests {
TEST_CASE("String Interning", "[memory]")
{
    using namespace poise::runtime::memory;

    const auto helloWorld = internString("Hello world");
    const auto helloWorld2 = internString("Hello world");

    REQUIRE(helloWorld == helloWorld2);

    std::vector<std::string> strings{
        "Foo",
        "Bar",
        "Baz",
        "Something reallyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy long",
        "",
        "Foo",
        "C:/Users/Me/tax-evasion.txt",
    };

    for (auto& s : strings) {
        [[maybe_unused]] const auto _ = internString(std::move(s));
    }

    REQUIRE(internedStringCount() == 7_uz);

    REQUIRE(removeInternedString("Foo"));
    REQUIRE(!removeInternedString("FooBar"));

    REQUIRE(internedStringCount() == 6_uz);
}
} // namespace poise::tests

