#include "compiler/Compiler.hpp"
#include "runtime/Vm.hpp"

#include <fmt/core.h>

#include <chrono>
#include <cstdlib>
#include <filesystem>

int main(int argc, const char* argv[])
{
    if (argc < 2) {
        fmt::print(stderr, "Expected file\n");
        std::exit(1);
    }

    const auto verbose = argc >= 3 && (std::strcmp(argv[std::size_t{2}], "--verbose") == 0 || std::strcmp(argv[std::size_t{2}], "-v") == 0);

    std::filesystem::path inFilePath{argv[std::size_t{1}]};

    if (!std::filesystem::exists(inFilePath)) {
        fmt::print(stderr, "File not found\n");
        std::exit(1);
    }

    if (inFilePath.extension() != ".poise") {
        fmt::print(stderr, "Invalid file type\n");
        std::exit(1);
    }

    poise::runtime::Vm vm{inFilePath.string()};
    poise::compiler::Compiler compiler{true, false, &vm, std::move(inFilePath)};

    {
        const auto start = std::chrono::steady_clock::now();
        auto compileResult = compiler.compile();
        const auto end = std::chrono::steady_clock::now();

        if (compileResult != poise::compiler::Compiler::CompileResult::Success) {
            if (compileResult == poise::compiler::Compiler::CompileResult::FileError) {
                fmt::print(stderr, "Invalid file\n");
            } else if (compileResult == poise::compiler::Compiler::CompileResult::ParseError) {
                fmt::print(stderr, "Invalid file content\n");
            }

            return static_cast<int>(compileResult);
        }

        if (verbose) {
            const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            fmt::print("Compilation finished in {} μs\n", duration);
        }
    }

    {
        const auto start = std::chrono::steady_clock::now();
        const auto res = static_cast<int>(vm.run());
        const auto end = std::chrono::steady_clock::now();

        if (verbose) {
            const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            if (duration > 1000) {
                fmt::print("Run finished in {} ms\n", static_cast<float>(duration) / 1000.0f);
            } else {
                fmt::print("Run finished in {} μs\n", duration);
            }
        }

        return res;
    }
}
