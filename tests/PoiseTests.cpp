#include "../src/compiler/Compiler.hpp"
#include "../src/objects/PoiseException.hpp"
#include "../src/runtime/Value.hpp"

#include <catch2/catch_test_macros.hpp>

namespace poise::tests
{
TEST_CASE("Compile and run 001_primitives.poise")
{
    runtime::Vm vm{"tests/test_files/001_primitives.poise"};
    compiler::Compiler compiler{true, false, &vm, "tests/test_files/001_primitives.poise"};
    REQUIRE(compiler.compile() == compiler::Compiler::CompileResult::Success);
    REQUIRE(vm.run(compiler.scanner()) == runtime::Vm::RunResult::Success);
}

TEST_CASE("Compile and run 002_local_variables.poise")
{
    runtime::Vm vm{"tests/test_files/002_local_variables.poise"};
    compiler::Compiler compiler{true, false, &vm, "tests/test_files/002_local_variables.poise"};
    REQUIRE(compiler.compile() == compiler::Compiler::CompileResult::Success);
    REQUIRE(vm.run(compiler.scanner()) == runtime::Vm::RunResult::Success);
}

TEST_CASE("Compile and run 003_functions.poise")
{
    runtime::Vm vm{"tests/test_files/003_functions.poise"};
    compiler::Compiler compiler{true, false, &vm, "tests/test_files/003_functions.poise"};
    REQUIRE(compiler.compile() == compiler::Compiler::CompileResult::Success);
    REQUIRE(vm.run(compiler.scanner()) == runtime::Vm::RunResult::Success);
}

TEST_CASE("Compile and run 004_types.poise")
{
    runtime::Vm vm{"tests/test_files/004_types.poise"};
    compiler::Compiler compiler{true, false, &vm, "tests/test_files/004_types.poise"};
    REQUIRE(compiler.compile() == compiler::Compiler::CompileResult::Success);
    REQUIRE(vm.run(compiler.scanner()) == runtime::Vm::RunResult::Success);
}

TEST_CASE("Compile and run 005_short_circuiting.poise")
{
    runtime::Vm vm{"tests/test_files/005_short_circuiting.poise"};
    compiler::Compiler compiler{true, false, &vm, "tests/test_files/005_short_circuiting.poise"};
    REQUIRE(compiler.compile() == compiler::Compiler::CompileResult::Success);
    REQUIRE(vm.run(compiler.scanner()) == runtime::Vm::RunResult::Success);
}

TEST_CASE("Compile and run 006_exceptions.poise")
{
    runtime::Vm vm{"tests/test_files/006_exceptions.poise"};
    compiler::Compiler compiler{true, false, &vm, "tests/test_files/006_exceptions.poise"};
    REQUIRE(compiler.compile() == compiler::Compiler::CompileResult::Success);
    REQUIRE(vm.run(compiler.scanner()) == runtime::Vm::RunResult::Success);
}

TEST_CASE("Compile and run 007_if_statements.poise")
{
    runtime::Vm vm{"tests/test_files/007_if_statements.poise"};
    compiler::Compiler compiler{true, false, &vm, "tests/test_files/007_if_statements.poise"};
    REQUIRE(compiler.compile() == compiler::Compiler::CompileResult::Success);
    REQUIRE(vm.run(compiler.scanner()) == runtime::Vm::RunResult::Success);
}

TEST_CASE("Compile and run 008_while_loops.poise")
{
    runtime::Vm vm{"tests/test_files/008_while_loops.poise"};
    compiler::Compiler compiler{true, false, &vm, "tests/test_files/008_while_loops.poise"};
    REQUIRE(compiler.compile() == compiler::Compiler::CompileResult::Success);
    REQUIRE(vm.run(compiler.scanner()) == runtime::Vm::RunResult::Success);
}

TEST_CASE("Compile and run 009_imports.poise")
{
    runtime::Vm vm{"tests/test_files/009_imports.poise"};
    compiler::Compiler compiler{true, false, &vm, "tests/test_files/009_imports.poise"};
    REQUIRE(compiler.compile() == compiler::Compiler::CompileResult::Success);
    REQUIRE(vm.run(compiler.scanner()) == runtime::Vm::RunResult::Success);
}

TEST_CASE("Check binary operations on Value class")
{
    using namespace poise::runtime;

    Value int1 = 1, int2 = 2;
    REQUIRE(int1 + int2 == Value{3});
    REQUIRE((int1 | int2) == Value{3});
    REQUIRE(~int1 == Value{-2});

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

TEST_CASE("Check some basic reference counting")
{
    using namespace poise::runtime;
    using namespace poise::objects;

    auto function = Value::createObject<PoiseFunction>("test", "", 0_uz, 0_u8, false);
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
}