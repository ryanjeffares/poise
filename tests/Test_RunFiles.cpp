//
// Created by ryand on 16/12/2023.
//

#include "../src/compiler/Compiler.hpp"

#include <catch2/catch_test_macros.hpp>

namespace poise::tests {
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

TEST_CASE("Compile and run 010_lists.poise")
{
    runtime::Vm vm{"tests/test_files/010_lists.poise"};
    compiler::Compiler compiler{true, false, &vm, "tests/test_files/010_lists.poise"};
    REQUIRE(compiler.compile() == compiler::Compiler::CompileResult::Success);
    REQUIRE(vm.run(compiler.scanner()) == runtime::Vm::RunResult::Success);
}

TEST_CASE("Compile and run 011_ranges.poise")
{
    runtime::Vm vm{"tests/test_files/011_ranges.poise"};
    compiler::Compiler compiler{true, false, &vm, "tests/test_files/011_ranges.poise"};
    REQUIRE(compiler.compile() == compiler::Compiler::CompileResult::Success);
    REQUIRE(vm.run(compiler.scanner()) == runtime::Vm::RunResult::Success);
}
}