//
// Created by ryand on 12/12/2023.
//

#ifndef POISE_NAMESPACE_MANAGER_HPP
#define POISE_NAMESPACE_MANAGER_HPP

#include "../Poise.hpp"

#include "Value.hpp"

#include <span>
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

    struct Constant
    {
        Value value;
        std::string name;
        bool isExported;
    };

    using NamespaceConstantLookup = std::unordered_map<NamespaceHash, std::vector<Constant>>;

    [[nodiscard]] auto namespaceHash(const std::filesystem::path& namespacePath) const noexcept -> NamespaceHash;

    [[nodiscard]] auto addNamespace(const std::filesystem::path& namespacePath, std::string namespaceName, std::optional<NamespaceHash> parent) noexcept -> bool;
    [[nodiscard]] auto namespaceDisplayName(NamespaceHash namespaceHash) const noexcept -> std::string_view;

    auto addFunctionToNamespace(NamespaceHash namespaceHash, Value function) noexcept -> void;
    [[nodiscard]] auto namespaceFunction(NamespaceHash namespaceHash, std::string_view functionName) const noexcept -> objects::Function*;
    [[nodiscard]] auto namespaceFunction(NamespaceHash namespaceHash, FunctionNameHash functionNameHash) const noexcept -> std::optional<runtime::Value>;
    [[nodiscard]] auto namespaceFunctions(NamespaceHash namespaceHash) const noexcept -> std::span<const runtime::Value>;

    [[nodiscard]] auto namespaceHasImportedNamespace(NamespaceHash parent, NamespaceHash imported) const noexcept -> bool;

    auto addConstant(NamespaceHash namespaceHash, Value value, std::string name, bool isExported) noexcept -> void;
    [[nodiscard]] auto hasConstant(NamespaceHash namespaceHash, std::string_view constantName) const noexcept -> bool;
    [[nodiscard]] auto getConstant(NamespaceHash namespaceHash, std::string_view constantName) const noexcept -> std::optional<Constant>;

private:
    std::hash<std::filesystem::path> m_namespaceHasher;

    NamespaceDisplayNameLookup m_namespaceDisplayNameMap;
    NamespaceFunctionLookup m_namespaceFunctionLookup;
    NamespacesImportedToNamespaceLookup m_namespacesImportedToNamespaceLookup;
    NamespaceConstantLookup m_namespaceConstantLookup;
};
}   // namespace poise::runtime

#endif  // #ifndef POISE_NAMESPACE_MANAGER_HPP
