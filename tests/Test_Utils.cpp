#include "../src/utils/DualIndexSet.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <functional>
#include <string>

namespace fs = std::filesystem;

struct CustomType
{
    fs::path path{};
    std::string data{};
};

template<>
struct std::hash<CustomType>
{
    [[nodiscard]] auto operator()(const CustomType& value) const -> std::size_t 
    {
        return std::hash<fs::path>{}(value.path);
    }
};

namespace poise::tests {
TEST_CASE("DualIndexSet<int>", "[utils]")
{
    using namespace poise::utils;

    DualIndexSet<int> set;

    for (auto i = 0; i < 50; i++) {
        set.insert(i);
    }

    REQUIRE(set.size() == 50_uz);

    for (auto i = 15; i < 25; i++) {
        REQUIRE(set.find(std::hash<int>{}(i)) == i);
    }

    REQUIRE(set.remove(10));
    REQUIRE(!set.remove(100));
    REQUIRE(set.size() == 49_uz);
}

TEST_CASE("DualIndexSet<std::string>", "[utils]")
{
    using namespace poise::utils;

    DualIndexSet<std::string> set;

    set.insert("Hello");
    set.insert("World");
    set.insert("Ryan");

    REQUIRE(set.size() == 3_uz);
    REQUIRE(set.remove("Hello"));
    REQUIRE(!set.remove("Foo"));
    REQUIRE(set.size() == 2_uz);

    REQUIRE(set.insert("Foo").hash == std::hash<std::string>{}("Foo"));
    REQUIRE(set.insert("Bar").hash == std::hash<std::string>{}("Bar"));
    REQUIRE(set.insert("Baz").hash == std::hash<std::string>{}("Baz"));
    REQUIRE(set.insert("Fizz").hash == std::hash<std::string>{}("Fizz"));
    REQUIRE(set.insert("Buzz").hash == std::hash<std::string>{}("Buzz"));

    REQUIRE(set.insert("Buzz").hash == std::hash<std::string>{}("Buzz"));
    REQUIRE(set.insert("Buzz").hash == std::hash<std::string>{}("Buzz"));
    REQUIRE(set.insert("Buzz").hash == std::hash<std::string>{}("Buzz"));
    REQUIRE(set.insert("Buzz").hash == std::hash<std::string>{}("Buzz"));

    REQUIRE(set.capacity() == 16_uz);
}

TEST_CASE("DualIndexSet<CustomType>", "[utils]")
{
    using namespace poise::utils;

    DualIndexSet<CustomType> set;

    auto data1 = CustomType{
        .path = fs::current_path(),
        .data = "Hello world!",
    };

    auto data2 = CustomType{
        .path = fs::current_path().parent_path(),
        .data = "Goodbye world :(",
    };

    REQUIRE(set.insert(data1).hash == std::hash<fs::path>{}(fs::current_path()));
    REQUIRE(set.insert(data2).hash == std::hash<CustomType>{}(data2));

    set.insert(CustomType{
        .path = fs::current_path(),
        .data = "Could this be a dog?",
    });

    REQUIRE(set.size() == 2_uz);
}
} // namespace poise::tests

