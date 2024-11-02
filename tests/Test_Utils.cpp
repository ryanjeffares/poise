#include "../src/utils/DualIndexSet.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <functional>
#include <string>

namespace fs = std::filesystem;

struct CustomType
{
    poise::usize pathHash{};
    fs::path path{};
    std::string data{};
};

template<>
struct boost::hash<CustomType>
{
    [[nodiscard]] auto operator()(const CustomType& value) const -> std::size_t 
    {
        return value.pathHash;
    }
};

namespace poise::tests {
TEST_CASE("DualIndexSet<int>", "[utils]")
{
    using namespace poise::utils;

    DualIndexSet<int> set;
    using Hasher = decltype(set)::Hasher;
    Hasher hasher;

    for (auto i = 0; i < 50; i++) {
        set.insert(i);
    }

    REQUIRE(set.size() == 50_uz);

    for (auto i = 15; i < 25; i++) {
        const auto it = set.find(hasher(i));
        REQUIRE(it != set.end());
        REQUIRE(it->second == i);
    }

    REQUIRE(set.remove(10));
    REQUIRE(!set.remove(100));
    REQUIRE(set.size() == 49_uz);
    REQUIRE(set.contains(20));
    REQUIRE(!set.contains(200));
}

TEST_CASE("DualIndexSet<std::string>", "[utils]")
{
    using namespace poise::utils;

    DualIndexSet<std::string> set;
    using Hasher = decltype(set)::Hasher;
    Hasher hasher;

    set.insert("Hello");
    set.insert("World");
    set.insert("Ryan");

    REQUIRE(set.size() == 3_uz);
    REQUIRE(set.remove(hasher("Hello")));
    REQUIRE(!set.remove(hasher("Foo")));
    REQUIRE(set.size() == 2_uz);

    REQUIRE(std::get<0>(set.insert("Foo")) == hasher("Foo"));
    REQUIRE(std::get<0>(set.insert("Bar")) == hasher("Bar"));
    REQUIRE(std::get<0>(set.insert("Baz")) == hasher("Baz"));
    REQUIRE(std::get<0>(set.insert("Fizz")) == hasher("Fizz"));
    REQUIRE(std::get<0>(set.insert("Buzz")) == hasher("Buzz"));
    REQUIRE(!std::get<2>(set.insert("Buzz")));
    REQUIRE(std::get<0>(set.insert("Buzz")) == hasher("Buzz"));
    REQUIRE(std::get<0>(set.insert("Buzz")) == hasher("Buzz"));
    REQUIRE(std::get<0>(set.insert("Buzz")) == hasher("Buzz"));
    REQUIRE(std::get<0>(set.insert("Buzz")) == hasher("Buzz"));

    REQUIRE(set.contains(hasher("Buzz")));
    REQUIRE(!set.contains(hasher("Bazz")));
}

TEST_CASE("DualIndexSet<CustomType>", "[utils]")
{
    using namespace poise::utils;

    DualIndexSet<CustomType> set;
    using Hasher = decltype(set)::Hasher;
    Hasher hasher;

    auto data1 = CustomType{
        .pathHash = boost::hash<fs::path>{}(fs::current_path()),
        .path = fs::current_path(),
        .data = "Hello world!",
    };

    auto data2 = CustomType{
        .pathHash = boost::hash<fs::path>{}(fs::current_path().parent_path()),
        .path = fs::current_path().parent_path(),
        .data = "Goodbye world :(",
    };

    REQUIRE(std::get<0>(set.insert(data1)) == boost::hash<fs::path>{}(fs::current_path()));
    REQUIRE(std::get<0>(set.insert(data2)) == hasher(data2));

    set.insert(CustomType{
        .pathHash = boost::hash<fs::path>{}(fs::current_path()),
        .path = fs::current_path(),
        .data = "Could this be a dog?",
    });

    REQUIRE(set.size() == 2_uz);
    REQUIRE(set.contains(hasher(data1)));
    REQUIRE(set.remove(hasher(data2)));
    REQUIRE(!set.contains(hasher(data2)));
    REQUIRE(set.remove(boost::hash<fs::path>{}(fs::current_path())));
    REQUIRE(set.empty());
}
} // namespace poise::tests

