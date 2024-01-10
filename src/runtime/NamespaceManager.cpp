//
// Created by ryand on 12/12/2023.
//

#include "NamespaceManager.hpp"
#include "../objects/PoiseFunction.hpp"

#include <algorithm>

namespace poise::runtime {
auto NamespaceManager::namespaceHash(const std::filesystem::path& namespacePath) const noexcept -> NamespaceHash
{
    return m_namespaceHasher(namespacePath);
}

auto NamespaceManager::addNamespace(const std::filesystem::path& namespacePath, std::string namespaceName, std::optional<NamespaceHash> parent) noexcept -> bool
{
    const auto hash = namespaceHash(namespacePath);

    // sort out namespaces' imported namespaces first
    // hacky....
    // in any case, this needs to be present in the map
    m_namespacesImportedToNamespaceLookup.try_emplace(hash, std::vector<NamespaceHash>{});
    if (parent) {
        // parent is only a nullopt if this was called from the constructor of the main file's compiler
        // so the parent file needs to have this import, but nothing imports the main file
        m_namespacesImportedToNamespaceLookup[*parent].push_back(hash);
    }

    // then return false if we've already compiled this file
    if (m_namespaceFunctionLookup.contains(hash)) {
        return false;
    }

    m_namespaceFunctionLookup.emplace(hash, std::vector<Value>{});
    m_namespaceDisplayNameMap[hash] = std::move(namespaceName);

    return true;
}

auto NamespaceManager::namespaceDisplayName(NamespaceHash namespaceHash) const noexcept -> std::string_view
{
    return m_namespaceDisplayNameMap.at(namespaceHash);
}

auto NamespaceManager::addFunctionToNamespace(NamespaceHash namespaceHash, Value function) noexcept -> void
{
    POISE_ASSERT(m_namespaceFunctionLookup.contains(namespaceHash), "Namespace not found");
    m_namespaceFunctionLookup[namespaceHash].emplace_back(std::move(function));
}

auto NamespaceManager::namespaceFunction(NamespaceHash namespaceHash, std::string_view functionName) const noexcept -> objects::Function*
{
    POISE_ASSERT(m_namespaceFunctionLookup.contains(namespaceHash), "Namespace not found");

    const auto& functionVec = m_namespaceFunctionLookup.at(namespaceHash);
    if (const auto it = std::find_if(functionVec.cbegin(), functionVec.cend(), [functionName] (const Value& value) {
        return value.object()->asFunction()->name() == functionName;
    }); it != functionVec.cend()) {
        return it->object()->asFunction();
    } else {
        return nullptr;
    }
}

auto NamespaceManager::namespaceFunction(NamespaceHash namespaceHash, FunctionNameHash functionNameHash) const noexcept -> std::optional<runtime::Value>
{
    const auto functions = namespaceFunctions(namespaceHash);
    if (const auto it = std::find_if(functions.begin(), functions.end(), [functionNameHash] (const Value& value) {
        return value.object()->asFunction()->nameHash() == functionNameHash;
    }); it != functions.end()) {
        return (*it);
    } else {
        return {};
    }
}

auto NamespaceManager::namespaceFunctions(NamespaceHash namespaceHash) const noexcept -> std::span<const runtime::Value>
{
    return m_namespaceFunctionLookup.at(namespaceHash);
}

auto NamespaceManager::namespaceHasImportedNamespace(NamespaceHash parent, NamespaceHash imported) const noexcept -> bool
{
    POISE_ASSERT(m_namespacesImportedToNamespaceLookup.contains(parent), "Parent namespace not found");
    const auto& namespaceVec = m_namespacesImportedToNamespaceLookup.at(parent);
    return std::find(namespaceVec.cbegin(), namespaceVec.cend(), imported) != namespaceVec.cend();
}
}   // namespace poise::runtime
