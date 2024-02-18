//
// Created by ryand on 12/12/2023.
//

#include "NamespaceManager.hpp"
#include "../objects/Function.hpp"

#include <algorithm>

namespace poise::runtime {
auto NamespaceManager::addNamespace(const std::filesystem::path& namespacePath, std::string namespaceName, std::optional<NamespaceHash> parent) noexcept -> bool
{
    // may or may not have already compiled this file, either way we need to look it up
    const auto [hash, inserted] = m_namespaceInfoLookup.insert(NamespaceInfo{
        .path = namespacePath,
        .displayName = std::move(namespaceName),
    });

    if (parent) {
        // parent is only a nullopt if this was called from the constructor of the main file's compiler
        // so the parent file needs to have this import, but nothing imports the main file
        m_namespaceInfoLookup.find(*parent).importedNamespaces.push_back(hash);
    }

    return inserted;
}

auto NamespaceManager::namespaceDisplayName(NamespaceHash namespaceHash) const noexcept -> std::string_view
{
    return m_namespaceInfoLookup.find(namespaceHash).displayName;
}

auto NamespaceManager::addFunctionToNamespace(NamespaceHash namespaceHash, Value function) noexcept -> void
{
    m_namespaceInfoLookup.find(namespaceHash).functions.emplace_back(std::move(function));
}

auto NamespaceManager::namespaceFunction(NamespaceHash namespaceHash, std::string_view functionName) const noexcept -> objects::Function*
{
    const auto functions = namespaceFunctions(namespaceHash);

    if (const auto it = std::ranges::find_if(functions, [functionName] (const Value& value) -> bool {
        return value.object()->asFunction()->name() == functionName;
    }); it != functions.end()) {
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
    return m_namespaceInfoLookup.find(namespaceHash).functions;
}

auto NamespaceManager::namespaceHasImportedNamespace(NamespaceHash parent, NamespaceHash imported) const noexcept -> bool
{
    const auto& namespaceVec = m_namespaceInfoLookup.find(parent).importedNamespaces;
    return std::ranges::find(namespaceVec, imported) != namespaceVec.end();
}

auto NamespaceManager::addConstant(NamespaceHash namespaceHash, Value value, std::string name, bool isExported) noexcept -> void
{
    m_namespaceInfoLookup.find(namespaceHash).constants.emplace_back(NamespaceConstant{
        std::move(value),
        std::move(name),
        isExported
    });
}

auto NamespaceManager::hasConstant(NamespaceHash namespaceHash, std::string_view constantName) const noexcept -> bool
{
    const auto& constantList = m_namespaceInfoLookup.find(namespaceHash).constants;
    return std::ranges::any_of(constantList, [&constantName] (const NamespaceConstant& constant) -> bool {
        return constant.name == constantName;
    });
}

auto NamespaceManager::getConstant(NamespaceHash namespaceHash, std::string_view constantName) const noexcept -> std::optional<NamespaceConstant>
{
    const auto& constantList = m_namespaceInfoLookup.find(namespaceHash).constants;
    const auto it = std::ranges::find_if(constantList, [constantName] (const NamespaceConstant& constant) -> bool {
        return constant.name == constantName;
    });

    if (it == constantList.end()) {
        return {};
    } else {
        return *it;
    }
}
}   // namespace poise::runtime

