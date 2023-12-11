#ifndef POISE_VM_HPP
#define POISE_VM_HPP

#include "../Poise.hpp"

#include "../objects/PoiseFunction.hpp"
#include "NativeFunction.hpp"
#include "Op.hpp"
#include "../scanner/Scanner.hpp"
#include "Value.hpp"

#include <unordered_map>
#include <utility>
#include <vector>

namespace poise::runtime {
class Vm
{
public:
    enum class RunResult
    {
        Success,
        InitError,
        RuntimeError,
    };

    using NativeNameHash = usize;
    using NativeFunctionMap = std::unordered_map<NativeNameHash, NativeFunction>;

    using NamespaceHash = usize;
    using NamespaceNameMap = std::unordered_map<NamespaceHash, std::string>;
    using NamespaceFunctionLookup = std::unordered_map<NamespaceHash, std::vector<Value>>;

    Vm(std::string mainFilePath);

    auto setCurrentFunction(objects::PoiseFunction* function) noexcept -> void;
    [[nodiscard]] auto currentFunction() const noexcept -> objects::PoiseFunction*;

    [[nodiscard]] auto nativeFunctionHash(std::string_view functionName) const noexcept -> std::optional<NativeNameHash>;
    [[nodiscard]] auto nativeFunctionArity(NativeNameHash hash) const noexcept -> u8;

    auto addNamespace(const std::filesystem::path& namespacePath, std::string namespaceName) noexcept -> void;
    [[nodiscard]] auto hasNamespace(const std::filesystem::path& namespacePath) const noexcept -> bool;
    [[nodiscard]] auto namespaceHash(const std::filesystem::path& namespacePath) const noexcept -> NamespaceHash;
    auto addFunctionToNamespace(const std::filesystem::path& namespacePath, Value function) noexcept -> void;
    [[nodiscard]] auto namespaceFunction(const std::filesystem::path& namespacePath, std::string_view functionName) const noexcept -> objects::PoiseFunction*;

    auto emitOp(Op op, usize line) noexcept -> void;
    auto emitConstant(Value value) noexcept -> void;

    [[nodiscard]] auto run(const scanner::Scanner* const scanner) noexcept -> RunResult;

private:
    auto registerNatives() noexcept -> void;
    auto registerIntNatives() noexcept -> void;

    std::string m_mainFilePath;

    std::vector<OpLine> m_globalOps;
    std::vector<Value> m_globalConstants;

    objects::PoiseFunction* m_currentFunction{nullptr};

    std::hash<std::string_view> m_nativeNameHasher;
    NativeFunctionMap m_nativeFunctionLookup;

    std::hash<std::filesystem::path> m_namespaceHasher;
    NamespaceNameMap m_namespaceNameMap;
    NamespaceFunctionLookup m_namespaceFunctionLookup;
};  // class Vm
}   // namespace poise::runtime

#endif  // #ifndef POISE_VM_HPP
