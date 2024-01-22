//
// Created by ryand on 16/12/2023.
//

#include "../src/objects/Objects.hpp"

#include <catch2/catch_test_macros.hpp>

namespace poise::tests {
TEST_CASE("List", "[objects]") 
{
    using namespace poise::runtime;
    using namespace poise::objects::iterables;

    List list{std::vector<runtime::Value>{}};
    REQUIRE(list.empty());
    Iterator emptyIterator{&list};
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

    Iterator iterator{&list};
    REQUIRE(iterator.value() == 2);

    for (auto i = 0; i < 10; i++) {
        iterator.increment();
    }

    REQUIRE(iterator.value() == 13);

    list.clear();
    REQUIRE(list.empty());
    REQUIRE(!iterator.valid());

    List fromString{"Hello"};
    REQUIRE(fromString.size() == 5_uz);
    List fromRange{Value::createObject<Range>(0, 10, 1, false)};
    REQUIRE(fromRange.size() == 10_uz);

    auto repeated = fromString.repeat(10);
    REQUIRE(repeated.object()->asList()->size() == 50_uz);
    auto concatenated = fromString.concat(fromRange);
    REQUIRE(concatenated.object()->asList()->size() == 15_uz);
}

TEST_CASE("Range", "[objects]") 
{
    using namespace poise::runtime;
    using namespace poise::objects::iterables;

    {
        Range range{0, 10, 1, false};
        REQUIRE(!range.isInfiniteLoop());
        REQUIRE(range.size() == 10_uz);

        Iterator iterator{&range};
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
        Range range{0, -10, 1, false};
        REQUIRE(range.isInfiniteLoop());
        REQUIRE(range.size() == 0_uz);

        Iterator iterator{&range};
        REQUIRE(iterator.isAtEnd());
    }

    {
        Range range{0, 10, 1, true};
        Iterator iterator{&range};

        for (auto i = 0; i < 10; i++) {
            iterator.increment();
        }

        REQUIRE(!iterator.isAtEnd());
        REQUIRE(iterator.value() == 10);

        iterator.increment();

        REQUIRE(iterator.isAtEnd());
    }
}

TEST_CASE("Tuple", "[objects]") 
{
    using namespace poise::runtime;
    using namespace poise::objects::iterables;

    Tuple tuple{std::vector<Value>{Value{0}, Value{"Hello"}, Value{true}}};
    REQUIRE(tuple.size() == 3_uz);
    REQUIRE(tuple.at(0_uz) == 0);

    Iterator iterator{&tuple};
    REQUIRE(!iterator.isAtEnd());

    iterator.increment();

    REQUIRE(iterator.value() == std::string{"Hello"});
}

TEST_CASE("Dictionary", "[objects]") 
{
    using namespace poise::runtime;
    using namespace poise::objects::iterables;
    using namespace poise::objects::iterables::hashables;

    std::vector<Value> pairs;
    pairs.emplace_back(Value::createObject<Tuple>("Ryan", 24));
    pairs.emplace_back(Value::createObject<Tuple>("Cat", 12));
    pairs.emplace_back(Value::createObject<Tuple>("Snake", 8));

    Dict dict{pairs};

    REQUIRE(dict.capacity() == Dict::s_initialCapacity);
    REQUIRE(dict.size() == 3_uz);
    REQUIRE(!dict.tryInsert("Ryan", 25));
    REQUIRE(dict.tryInsert("Hotel", "Trivago"));
    REQUIRE(dict.at("Ryan") == 24);
    REQUIRE(dict.at("Cat") == 12);
    REQUIRE(dict.at("Snake") == 8);
    REQUIRE(dict.at("Hotel") == "Trivago");

    dict.insertOrUpdate("Ryan2", 24);
    dict.insertOrUpdate("Cat2", 12);
    dict.insertOrUpdate("Snake2", 8);
    dict.insertOrUpdate("Ryan3", 24);
    dict.insertOrUpdate("Cat3", 12);
    dict.insertOrUpdate("Snake3", 8);

    REQUIRE(dict.capacity() == Hashable::s_initialCapacity * 2_uz);
    REQUIRE(dict.size() == 10_uz);

    REQUIRE(dict.remove("Ryan2"));
    REQUIRE(!dict.remove("Foo"));

    Iterator iterator{&dict};
    REQUIRE(!iterator.isAtEnd());
    iterator.increment();
    REQUIRE(!iterator.isAtEnd());
    for (auto i = 0_uz; i < dict.size() - 1_uz; i++) {
        iterator.increment();
    }
    REQUIRE(iterator.isAtEnd());
}

TEST_CASE("Set", "[objects]") 
{
    using namespace poise::runtime;
    using namespace poise::objects::iterables;
    using namespace poise::objects::iterables::hashables;

    std::vector<Value> names{
        "Ryan",
        "Cat",
        "Snake",
    };

    Set set{names};

    REQUIRE(set.capacity() == Hashable::s_initialCapacity);
    REQUIRE(set.size() == 3_uz);
    REQUIRE(!set.tryInsert("Ryan"));
    REQUIRE(set.tryInsert("Hotel"));

    REQUIRE(set.contains("Ryan"));
    REQUIRE(set.contains("Cat"));
    REQUIRE(set.contains("Snake"));
    REQUIRE(set.contains("Hotel"));
    REQUIRE(!set.contains(0));

    set.tryInsert(true);
    set.tryInsert(false);
    set.tryInsert(3.14);

    REQUIRE(set.capacity() == Dict::s_initialCapacity * 2_uz);
    REQUIRE(set.size() == 7_uz);

    Iterator iterator{&set};
    REQUIRE(!iterator.isAtEnd());
    iterator.increment();
    REQUIRE(!iterator.isAtEnd());
    for (auto i = 0_uz; i < set.size() - 1_uz; i++) {
        iterator.increment();
    }
    REQUIRE(iterator.isAtEnd());

    std::vector<Value> nums = {0, 1, 2};
    Set a{nums};
    nums.emplace_back(3);
    nums.emplace_back(4);
    nums.emplace_back(5);
    Set b{nums};

    REQUIRE(a.isSubset(b));
    REQUIRE(b.isSuperset(a));
    REQUIRE(a.isSubset(a));
    REQUIRE(b.isSuperset(b));

    REQUIRE(a.unionWith(b).object()->asSet()->size() == 6_uz);
    REQUIRE(a.intersection(b).object()->asSet()->size() == 3_uz);
    REQUIRE(b.difference(a).object()->asSet()->size() == 3_uz);

    a.tryInsert(-1);
    a.tryInsert(-2);
    a.tryInsert(-3);

    REQUIRE(a.symmetricDifference(b).object()->asSet()->size() == 6_uz);

    REQUIRE(a.remove(-1));
    REQUIRE(!a.remove("Foo"));

    REQUIRE(Set{"RyanRyanRyan"}.size() == 4_uz);

    names = {
        "Ryan",
        "Cat",
        "Snake",
    };
    names.emplace_back("Ryan");
    auto list = Value::createObject<List>(names);
    REQUIRE(Set{list}.size() == 3_uz);

    auto pairs = std::vector<Value>{
        Value::createObject<Tuple>("Ryan", 24),
        Value::createObject<Tuple>("Puffle", 12),
        Value::createObject<Tuple>("Noodle", 8),
    };
    auto dict = Value::createObject<Dict>(pairs);

    REQUIRE(Set{dict}.size() == 3_uz);

    auto range = Value::createObject<Range>(0, 20, 1, false);
    REQUIRE(Set{range}.size() == 20_uz);

    REQUIRE(Set{0}.size() == 1_uz);
}
} // namespace poise::tests

