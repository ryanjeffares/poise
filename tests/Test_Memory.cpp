#include "Test_Macros.hpp"
#include "../src/objects/Objects.hpp"
#include "../src/runtime/memory/StringInterner.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

namespace poise::tests {
TEST_CASE("Basic Reference Counting", "[memory]")
{
    using namespace poise::runtime;
    using namespace poise::objects;

    REINITIALISE();

    const auto function = Value::createObject<Function>("test", "", 0_uz, 0_u8, false, false);
    const auto exception = Value::createObject<Exception>("Test");

    {
        function.object()->asFunction()->addCapture(exception);
        const auto functionCopy = function;
        const auto exceptionCopy = exception;
        functionCopy.print(false, true);
        exceptionCopy.print(false, true);
    }

    REQUIRE((function.object()->refCount() == 1_uz && exception.object()->refCount() == 2_uz));
}

TEST_CASE("String Interning", "[memory]")
{
    using namespace poise::runtime::memory;
 
    REINITIALISE();

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

