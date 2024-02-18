//
// Created by ryand on 12/12/2023.
//

#ifndef POISE_NAMESPACE_MANAGER_HPP
#define POISE_NAMESPACE_MANAGER_HPP

#include "../Poise.hpp"
#include "../utils/DualIndexSet.hpp"
#include "Value.hpp"

#include <span>
#include <vector>

namespace poise::runtime {
struct NamespaceConstant
{
    Value value;
    std::string name;
    bool isExported;
};

struct NamespaceInfo
{
    std::filesystem::path path{};
    std::string displayName{};
    std::vector<Value> functions{};
    std::vector<NamespaceConstant> constants{};
    std::vector<usize> importedNamespaces{};
};
}

template<>
struct std::hash<poise::runtime::NamespaceInfo>
{
    [[nodiscard]] auto operator()(const poise::runtime::NamespaceInfo& namespaceInfo) const -> std::size_t
    {
        return std::hash<std::filesystem::path>{}(namespaceInfo.path);
    }
};

namespace poise::runtime {
class NamespaceManager
{
public:
    using FunctionNameHash = usize;
    using NamespaceHash = usize;

    // returns wether this is a newly added namespace, or one that's already been compiled.
    // in either case, it will add `namespacePath` to `parent`'s imported namespaces
    [[nodiscard]] auto addNamespace(const std::filesystem::path& namespacePath, std::string namespaceName, std::optional<NamespaceHash> parent) noexcept -> bool;
    [[nodiscard]] auto namespaceDisplayName(NamespaceHash namespaceHash) const noexcept -> std::string_view;

    auto addFunctionToNamespace(NamespaceHash namespaceHash, Value function) noexcept -> void;
    [[nodiscard]] auto namespaceFunction(NamespaceHash namespaceHash, std::string_view functionName) const noexcept -> objects::Function*;
    [[nodiscard]] auto namespaceFunction(NamespaceHash namespaceHash, FunctionNameHash functionNameHash) const noexcept -> std::optional<runtime::Value>;
    [[nodiscard]] auto namespaceFunctions(NamespaceHash namespaceHash) const noexcept -> std::span<const runtime::Value>;

    [[nodiscard]] auto namespaceHasImportedNamespace(NamespaceHash parent, NamespaceHash imported) const noexcept -> bool;

    auto addConstant(NamespaceHash namespaceHash, Value value, std::string name, bool isExported) noexcept -> void;
    [[nodiscard]] auto hasConstant(NamespaceHash namespaceHash, std::string_view constantName) const noexcept -> bool;
    [[nodiscard]] auto getConstant(NamespaceHash namespaceHash, std::string_view constantName) const noexcept -> std::optional<NamespaceConstant>;

private:
    utils::DualIndexSet<NamespaceInfo> m_namespaceInfoLookup;
};
}   // namespace poise::runtime

#endif  // #ifndef POISE_NAMESPACE_MANAGER_HPP

