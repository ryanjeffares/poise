#include "../src/compiler/Compiler.hpp"

#include <catch2/catch_test_macros.hpp>

namespace poise::tests
{
    TEST_CASE("Compile and run 001_hello_world.poise") {
        runtime::Vm vm;
        compiler::Compiler compiler{&vm, "tests/test_files/001_hello_world.poise"};
        REQUIRE(compiler.compile() == compiler::CompileResult::Success);
        REQUIRE(vm.run() == runtime::Vm::RunResult::Success);
    }

    TEST_CASE("Compile and run 002_int.poise") {
        runtime::Vm vm;
        compiler::Compiler compiler{&vm, "tests/test_files/002_int.poise"};
        REQUIRE(compiler.compile() == compiler::CompileResult::Success);
        REQUIRE(vm.run() == runtime::Vm::RunResult::Success);
    }

    TEST_CASE("Compile and run 003_float.poise") {
        runtime::Vm vm;
        compiler::Compiler compiler{&vm, "tests/test_files/003_float.poise"};
        REQUIRE(compiler.compile() == compiler::CompileResult::Success);
        REQUIRE(vm.run() == runtime::Vm::RunResult::Success);
    }

    TEST_CASE("Compile and run 004_none.poise") {
        runtime::Vm vm;
        compiler::Compiler compiler{&vm, "tests/test_files/004_none.poise"};
        REQUIRE(compiler.compile() == compiler::CompileResult::Success);
        REQUIRE(vm.run() == runtime::Vm::RunResult::Success);
    }

    TEST_CASE("Compile and run 005_bool.poise") {
        runtime::Vm vm;
        compiler::Compiler compiler{&vm, "tests/test_files/005_bool.poise"};
        REQUIRE(compiler.compile() == compiler::CompileResult::Success);
        REQUIRE(vm.run() == runtime::Vm::RunResult::Success);
    }
}