#include "compiler/Compiler.hpp"

#include <fmt/core.h>

#include <cstdlib>
#include <filesystem>

int main([[maybe_unused]] int argc, [[maybe_unused]] const char* argv[])
{
    if (argc < 2) {
        fmt::print(stderr, "Expected file\n");
        std::exit(1);
    }

    std::filesystem::path inFilePath{argv[1]};

    if (!std::filesystem::exists(inFilePath)) {
        fmt::print(stderr, "File not found\n");
        std::exit(1);
    }

    if (inFilePath.extension() != ".poise") {
        fmt::print(stderr, "Invalid file type\n");
        std::exit(1);
    }

    poise::compiler::Compiler compiler{std::move(inFilePath)};
    compiler.compile();
}
