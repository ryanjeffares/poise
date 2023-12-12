//
// Created by ryand on 12/12/2023.
//

#ifndef POISE_NAMESPACE_MANAGER_HPP
#define POISE_NAMESPACE_MANAGER_HPP

#include "../Poise.hpp"

#include "NativeFunction.hpp"
#include "Value.hpp"

#include <unordered_map>
#include <vector>

namespace poise::runtime {
class NamespaceManager
{
public:
    using FunctionNameHash = usize;

    using NamespaceHash = usize;
    using NamespaceDisplayNameLookup = std::unordered_map<NamespaceHash, std::string>;
    using NamespaceFunctionLookup = std::unordered_map<NamespaceHash, std::vector<Value>>;
    using NamespacesImportedToNamespaceLookup = std::unordered_map<NamespaceHash, std::vector<NamespaceHash>>;

    [[nodiscard]] auto namespaceHash(const std::filesystem::path& namespaceHash) const noexcept -> NamespaceHash;

    [[nodiscard]] auto addNamespace(const std::filesystem::path& namespacePath, std::string namespaceName, std::optional<NamespaceHash> parent) noexcept -> bool;
    [[nodiscard]] auto namespaceDisplayName(NamespaceHash namespaceHash) const noexcept -> std::string_view;

    auto addFunctionToNamespace(NamespaceHash namespaceHash, Value function) noexcept -> void;
    [[nodiscard]] auto namespaceFunction(NamespaceHash namespaceHash, std::string_view functionName) const noexcept -> objects::PoiseFunction*;
    [[nodiscard]] auto namespaceFunction(NamespaceHash namespaceHash, FunctionNameHash functionNameHash) const noexcept -> std::optional<runtime::Value>;
    [[nodiscard]] auto namespaceFunctions(NamespaceHash namespaceHash) const noexcept -> std::span<const runtime::Value>;

    [[nodiscard]] auto namespaceHasImportedNamespace(NamespaceHash parent, NamespaceHash imported) const noexcept -> bool;

private:
    std::hash<std::filesystem::path> m_namespaceHasher;

    NamespaceDisplayNameLookup m_namespaceDisplayNameMap;
    NamespaceFunctionLookup m_namespaceFunctionLookup;

    NamespacesImportedToNamespaceLookup m_namespacesImportedToNamespaceLookup;
};
}   // namespace poise::runtime

#endif  // #ifndef POISE_NAMESPACE_MANAGER_HPP
