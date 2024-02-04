//
// Created by ryand on 12/12/2023.
//

#include "NamespaceManager.hpp"
#include "../objects/Function.hpp"

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
    m_namespaceConstantLookup.emplace(hash, std::vector<Constant>{});

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
    if (const auto it = std::ranges::find_if(functionVec, [functionName] (const Value& value) -> bool {
        return value.object()->asFunction()->name() == functionName;
    }); it != functionVec.end()) {
        return it->object()->asFunction();
    }

    return nullptr;
}

auto NamespaceManager::namespaceFunction(NamespaceHash namespaceHash, FunctionNameHash functionNameHash) const noexcept -> std::optional<Value>
{
    const auto functions = namespaceFunctions(namespaceHash);
    if (const auto it = std::ranges::find_if(functions, [functionNameHash] (const Value& value) -> bool {
        return value.object()->asFunction()->nameHash() == functionNameHash;
    }); it != functions.end()) {
        return *it;
    }

    return {};
}

auto NamespaceManager::namespaceFunctions(NamespaceHash namespaceHash) const noexcept -> std::span<const Value>
{
    return m_namespaceFunctionLookup.at(namespaceHash);
}

auto NamespaceManager::namespaceHasImportedNamespace(NamespaceHash parent, NamespaceHash imported) const noexcept -> bool
{
    POISE_ASSERT(m_namespacesImportedToNamespaceLookup.contains(parent), "Parent namespace not found");
    const auto& namespaceVec = m_namespacesImportedToNamespaceLookup.at(parent);
    return std::ranges::find(namespaceVec, imported) != namespaceVec.end();
}

auto NamespaceManager::addConstant(NamespaceHash namespaceHash, Value value, std::string name) noexcept -> void
{
    m_namespaceConstantLookup.at(namespaceHash).emplace_back(Constant{std::move(value), std::move(name)});
}

auto NamespaceManager::hasConstant(NamespaceHash namespaceHash, std::string_view constantName) const noexcept -> bool
{
    const auto& constantList = m_namespaceConstantLookup.at(namespaceHash);
    return std::ranges::any_of(constantList, [&constantName] (const Constant& constant) -> bool {
        return constant.name == constantName;
    });
}

auto NamespaceManager::getConstant(NamespaceHash namespaceHash, std::string_view constantName) const noexcept -> std::optional<Value>
{
    const auto& constantList = m_namespaceConstantLookup.at(namespaceHash);
    const auto it = std::ranges::find_if(constantList, [constantName] (const Constant& constant) -> bool {
        return constant.name == constantName;
    });

    if (it == constantList.end()) {
        return {};
    } else {
        return it->value;
    }
}
}   // namespace poise::runtime

