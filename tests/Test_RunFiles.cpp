//
// Created by ryand on 16/12/2023.
//

#include "Test_Macros.hpp"
#include "../src/compiler/Compiler.hpp"

#include <catch2/catch_test_macros.hpp>

namespace poise::tests {
TEST_CASE("001_primitives.poise", "[files]")
{
    REINITIALISE();

    runtime::Vm vm{"tests/test_files/001_primitives.poise"};
    compiler::Compiler compiler{true, false, &vm, "tests/test_files/001_primitives.poise"};
    REQUIRE(compiler.compile() == compiler::Compiler::CompileResult::Success);
    REQUIRE(vm.run() == runtime::Vm::RunResult::Success);
}

TEST_CASE("002_local_variables.poise", "[files]")
{
    REINITIALISE();

    runtime::Vm vm{"tests/test_files/002_local_variables.poise"};
    compiler::Compiler compiler{true, false, &vm, "tests/test_files/002_local_variables.poise"};
    REQUIRE(compiler.compile() == compiler::Compiler::CompileResult::Success);
    REQUIRE(vm.run() == runtime::Vm::RunResult::Success);
}

TEST_CASE("003_functions.poise", "[files]")
{
    REINITIALISE();

    runtime::Vm vm{"tests/test_files/003_functions.poise"};
    compiler::Compiler compiler{true, false, &vm, "tests/test_files/003_functions.poise"};
    REQUIRE(compiler.compile() == compiler::Compiler::CompileResult::Success);
    REQUIRE(vm.run() == runtime::Vm::RunResult::Success);
}

TEST_CASE("004_types.poise", "[files]")
{
    REINITIALISE();

    runtime::Vm vm{"tests/test_files/004_types.poise"};
    compiler::Compiler compiler{true, false, &vm, "tests/test_files/004_types.poise"};
    REQUIRE(compiler.compile() == compiler::Compiler::CompileResult::Success);
    REQUIRE(vm.run() == runtime::Vm::RunResult::Success);
}

TEST_CASE("005_short_circuiting.poise", "[files]")
{
    REINITIALISE();

    runtime::Vm vm{"tests/test_files/005_short_circuiting.poise"};
    compiler::Compiler compiler{true, false, &vm, "tests/test_files/005_short_circuiting.poise"};
    REQUIRE(compiler.compile() == compiler::Compiler::CompileResult::Success);
    REQUIRE(vm.run() == runtime::Vm::RunResult::Success);
}

TEST_CASE("006_exceptions.poise", "[files]")
{
    REINITIALISE();

    runtime::Vm vm{"tests/test_files/006_exceptions.poise"};
    compiler::Compiler compiler{true, false, &vm, "tests/test_files/006_exceptions.poise"};
    REQUIRE(compiler.compile() == compiler::Compiler::CompileResult::Success);
    REQUIRE(vm.run() == runtime::Vm::RunResult::Success);
}

TEST_CASE("007_if_statements.poise", "[files]")
{
    REINITIALISE();

    runtime::Vm vm{"tests/test_files/007_if_statements.poise"};
    compiler::Compiler compiler{true, false, &vm, "tests/test_files/007_if_statements.poise"};
    REQUIRE(compiler.compile() == compiler::Compiler::CompileResult::Success);
    REQUIRE(vm.run() == runtime::Vm::RunResult::Success);
}

TEST_CASE("008_while_loops.poise", "[files]")
{
    REINITIALISE();

    runtime::Vm vm{"tests/test_files/008_while_loops.poise"};
    compiler::Compiler compiler{true, false, &vm, "tests/test_files/008_while_loops.poise"};
    REQUIRE(compiler.compile() == compiler::Compiler::CompileResult::Success);
    REQUIRE(vm.run() == runtime::Vm::RunResult::Success);
}

TEST_CASE("009_imports.poise", "[files]")
{
    REINITIALISE();

    runtime::Vm vm{"tests/test_files/009_imports.poise"};
    compiler::Compiler compiler{true, false, &vm, "tests/test_files/009_imports.poise"};
    REQUIRE(compiler.compile() == compiler::Compiler::CompileResult::Success);
    REQUIRE(vm.run() == runtime::Vm::RunResult::Success);
}

TEST_CASE("010_lists.poise", "[files]")
{
    REINITIALISE();

    runtime::Vm vm{"tests/test_files/010_lists.poise"};
    compiler::Compiler compiler{true, false, &vm, "tests/test_files/010_lists.poise"};
    REQUIRE(compiler.compile() == compiler::Compiler::CompileResult::Success);
    REQUIRE(vm.run() == runtime::Vm::RunResult::Success);
}

TEST_CASE("011_ranges.poise", "[files]")
{
    REINITIALISE();

    runtime::Vm vm{"tests/test_files/011_ranges.poise"};
    compiler::Compiler compiler{true, false, &vm, "tests/test_files/011_ranges.poise"};
    REQUIRE(compiler.compile() == compiler::Compiler::CompileResult::Success);
    REQUIRE(vm.run() == runtime::Vm::RunResult::Success);
}

TEST_CASE("012_packs.poise", "[files]")
{
    REINITIALISE();

    runtime::Vm vm{"tests/test_files/012_packs.poise"};
    compiler::Compiler compiler{true, false, &vm, "tests/test_files/012_packs.poise"};
    REQUIRE(compiler.compile() == compiler::Compiler::CompileResult::Success);
    REQUIRE(vm.run() == runtime::Vm::RunResult::Success);
}

TEST_CASE("013_tuples.poise", "[files]")
{
    REINITIALISE();

    runtime::Vm vm{"tests/test_files/013_tuples.poise"};
    compiler::Compiler compiler{true, false, &vm, "tests/test_files/013_tuples.poise"};
    REQUIRE(compiler.compile() == compiler::Compiler::CompileResult::Success);
    REQUIRE(vm.run() == runtime::Vm::RunResult::Success);
}

TEST_CASE("014_dicts.poise", "[files]")
{
    REINITIALISE();

    runtime::Vm vm{"tests/test_files/014_dicts.poise"};
    compiler::Compiler compiler{true, false, &vm, "tests/test_files/014_dicts.poise"};
    REQUIRE(compiler.compile() == compiler::Compiler::CompileResult::Success);
    REQUIRE(vm.run() == runtime::Vm::RunResult::Success);
}

TEST_CASE("015_sets.poise", "[files]")
{
    REINITIALISE();

    runtime::Vm vm{"tests/test_files/015_sets.poise"};
    compiler::Compiler compiler{true, false, &vm, "tests/test_files/015_sets.poise"};
    REQUIRE(compiler.compile() == compiler::Compiler::CompileResult::Success);
    REQUIRE(vm.run() == runtime::Vm::RunResult::Success);
}

TEST_CASE("016_cycles.poise", "[files]")
{
    REINITIALISE();

    runtime::Vm vm{"tests/test_files/016_cycles.poise"};
    compiler::Compiler compiler{true, false, &vm, "tests/test_files/016_cycles.poise"};
    REQUIRE(compiler.compile() == compiler::Compiler::CompileResult::Success);
    REQUIRE(vm.run() == runtime::Vm::RunResult::Success);
}

TEST_CASE("017_constants.poise", "[files]")
{
    REINITIALISE();

    runtime::Vm vm{"tests/test_files/017_constants.poise"};
    compiler::Compiler compiler{true, false, &vm, "tests/test_files/017_constants.poise"};
    REQUIRE(compiler.compile() == compiler::Compiler::CompileResult::Success);
    REQUIRE(vm.run() == runtime::Vm::RunResult::Success);
}
} // namespace poise::tests

