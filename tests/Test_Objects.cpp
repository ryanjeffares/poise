//
// Created by ryand on 16/12/2023.
//

#include "../src/objects/PoiseException.hpp"
#include "../src/objects/PoiseFunction.hpp"
#include "../src/objects/iterables/PoiseList.hpp"
#include "../src/objects/iterables/PoiseRange.hpp"

#include <catch2/catch_test_macros.hpp>

namespace poise::tests {
TEST_CASE("Check some basic reference counting")
{
    using namespace poise::runtime;
    using namespace poise::objects;

    auto function = Value::createObject<PoiseFunction>("test", "", 0_uz, 0_u8, false, false);
    auto exception = Value::createObject<PoiseException>("Test");

    {
        function.object()->asFunction()->addCapture(exception);
        const auto functionCopy = function;
        const auto exceptionCopy = exception;
        functionCopy.print(false, true);
        exceptionCopy.print(false, true);
    }

    REQUIRE((function.object()->refCount() == 1_uz && exception.object()->refCount() == 2_uz));
}

TEST_CASE("PoiseList functions and iteration")
{
    using namespace poise::runtime;
    using namespace poise::objects::iterables;

    PoiseList list{std::vector<runtime::Value>{}};
    REQUIRE(list.empty());
    PoiseIterator emptyIterator{&list};
    REQUIRE(emptyIterator.isAtEnd());

    for (auto i = 0; i < 20; i++) {
        list.append(i);
    }

    REQUIRE(!emptyIterator.valid());
    REQUIRE(list.size() == 20_uz);

    REQUIRE(list.remove(10) == 1);
    REQUIRE(list.size() == 19);

    REQUIRE(list.at(10_uz) == 11);
    list.at(0_uz) = "Hello";
    REQUIRE(list.at(0_uz) == "Hello");

    REQUIRE(list.removeFirst("Hello"));
    REQUIRE(list.at(0_uz) == 1);
    REQUIRE(list.removeAt(0));
    REQUIRE(list.at(0_uz) == 2);

    PoiseIterator iterator{&list};
    REQUIRE(iterator.value() == 2);

    for (auto i = 0; i < 10; i++) {
        iterator.increment();
    }

    REQUIRE(iterator.value() == 13);

    list.clear();
    REQUIRE(list.empty());
    REQUIRE(!iterator.valid());

    PoiseList fromString{"Hello"};
    REQUIRE(fromString.size() == 5_uz);
    PoiseList fromRange{Value::createObject<PoiseRange>(0, 10, 1, false)};
    REQUIRE(fromRange.size() == 10_uz);
}

TEST_CASE("PoiseRange functions and iteration")
{
    using namespace poise::runtime;
    using namespace poise::objects::iterables;

    {
        PoiseRange range{0, 10, 1, false};
        REQUIRE(!range.isInfiniteLoop());

        PoiseIterator iterator{&range};
        REQUIRE(!iterator.isAtEnd());
        REQUIRE(iterator.valid());

        for (auto i = 0; i < 5; i++) {
            iterator.increment();
        }

        REQUIRE(!iterator.isAtEnd());
        REQUIRE(iterator.value() == 5);

        for (auto i = 0; i < 5; i++) {
            iterator.increment();
        }

        REQUIRE(iterator.isAtEnd());
    }

    {
        PoiseRange range{0, -10, 1, false};
        REQUIRE(range.isInfiniteLoop());

        PoiseIterator iterator{&range};
        REQUIRE(iterator.isAtEnd());
    }

    {
        PoiseRange range{0, 10, 1, true};
        PoiseIterator iterator{&range};

        for (auto i = 0; i < 10; i++) {
            iterator.increment();
        }

        REQUIRE(!iterator.isAtEnd());
        REQUIRE(iterator.value() == 10);

        iterator.increment();

        REQUIRE(iterator.isAtEnd());
    }
}
}