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
    using NamespacesImportedToNamespaceLookup = std::unordered_map<NamespaceHash, std::vector<NamespaceHash>>;

    Vm(std::string mainFilePath);

    auto setCurrentFunction(objects::PoiseFunction* function) noexcept -> void;
    [[nodiscard]] auto currentFunction() const noexcept -> objects::PoiseFunction*;

    [[nodiscard]] auto nativeFunctionHash(std::string_view functionName) const noexcept -> std::optional<NativeNameHash>;
    [[nodiscard]] auto nativeFunctionArity(NativeNameHash hash) const noexcept -> u8;

    [[nodiscard]] auto addNamespace(const std::filesystem::path& namespacePath, std::string namespaceName, std::optional<NamespaceHash> parent) noexcept -> bool;
    [[nodiscard]] auto hasNamespace(NamespaceHash namespaceHash) const noexcept -> bool;
    [[nodiscard]] auto namespaceHash(const std::filesystem::path& namespaceHash) const noexcept -> NamespaceHash;
    auto addFunctionToNamespace(NamespaceHash namespaceHash, Value function) noexcept -> void;
    [[nodiscard]] auto namespaceFunction(NamespaceHash namespaceHash, std::string_view functionName) const noexcept -> objects::PoiseFunction*;
    [[nodiscard]] auto namespaceHasImportedNamespace(NamespaceHash parent, NamespaceHash imported) const noexcept -> bool;

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
    NamespacesImportedToNamespaceLookup m_namespacesImportedToNamespaceLookup;
};  // class Vm
}   // namespace poise::runtime

#endif  // #ifndef POISE_VM_HPP
