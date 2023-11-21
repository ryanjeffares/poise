#include "compiler/Compiler.hpp"
#include "runtime/Vm.hpp"

#include <fmt/core.h>

#include <cstdlib>
#include <filesystem>

int main(int argc, const char* argv[])
{
    if (argc < 2) {
        fmt::print(stderr, "Expected file\n");
        std::exit(1);
    }

    std::filesystem::path inFilePath{argv[1zu]};

    if (!std::filesystem::exists(inFilePath)) {
        fmt::print(stderr, "File not found\n");
        std::exit(1);
    }

    if (inFilePath.extension() != ".poise") {
        fmt::print(stderr, "Invalid file type\n");
        std::exit(1);
    }

    poise::runtime::Vm vm;
    poise::compiler::Compiler compiler{&vm, std::move(inFilePath)};

    auto compileResult = compiler.compile();
    if (compileResult != poise::compiler::CompileResult::Success) {
        return static_cast<int>(compileResult);
    }

    return static_cast<int>(vm.run());
}
