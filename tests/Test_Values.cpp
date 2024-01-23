#include "Test_Macros.hpp"
#include "../src/runtime/Value.hpp"

#include <catch2/catch_test_macros.hpp>

namespace poise::tests
{
TEST_CASE("Binary Operations", "[values]")
{
    using namespace poise::runtime;

    REINITIALISE();

    Value int1 = 1, int2 = 2;
    REQUIRE(int1 + int2 == 3);
    REQUIRE((int1 | int2) == 3);
    REQUIRE(~int1 == -2);
    REQUIRE(int1 << 1 == 2);
    REQUIRE(int2 >> 1 == 1);

    Value float1 = 0.5, float2 = 3.2;
    REQUIRE(float1 + float2 == Value{3.7});
    REQUIRE(int1 * float1 == Value{0.5});

    Value bool1 = true, bool2 = false;
    REQUIRE((bool1 && bool2) == false);
    REQUIRE((bool1 || bool2) == true);
    REQUIRE(bool1 == true);

    Value none = nullptr;
    REQUIRE((none == bool1) == false);
    REQUIRE((none == int2) == false);

    Value str1 = "Hello", str2 = "World";
    REQUIRE(str1 + str2 == "HelloWorld");
    REQUIRE(str1 * int2 == "HelloHello");
}
} // namespace poise::tests

