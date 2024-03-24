#include "Test_Macros.hpp"
#include "../src/objects/Objects.hpp"
#include "../src/runtime/memory/Gc.hpp"
#include "../src/runtime/memory/StringInterner.hpp"
#include "../src/runtime/Value.hpp"

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
    }

    REQUIRE((function.object()->refCount() == 1_uz && exception.object()->refCount() == 2_uz));
}

TEST_CASE("Cyclic Reference Countring", "[memory]")
{
    using namespace poise::objects;
    using namespace poise::objects::iterables;
    using namespace poise::runtime;
    using namespace poise::runtime::memory;

    REINITIALISE();

    {
        auto list = Value::createObject<List>(std::vector<Value>{});
        list.object()->asList()->append(list);

        auto list2 = Value::createObject<List>(list);
        list.object()->asList()->append(list2);

        auto list3 = Value::createObject<List>(std::vector<Value>{list2, list});
        list3.object()->asList()->append(list3);
        list2.object()->asList()->append(list3);

        auto list4 = Value::createObject<List>(std::vector<Value>{list, list2, list3});
        list4.object()->asList()->append(list4);
        list.object()->asList()->append(list4);
        list2.object()->asList()->append(list4);
    }

    Gc::instance().cleanCycles();

    REQUIRE(Gc::instance().numTrackedObjects() == 0_uz);
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

