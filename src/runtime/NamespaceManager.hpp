//
// Created by ryand on 12/12/2023.
//

#ifndef POISE_NAMESPACE_MANAGER_HPP
#define POISE_NAMESPACE_MANAGER_HPP

#include "../Poise.hpp"
#include "../utils/DualIndexSet.hpp"
#include "Value.hpp"

#include <vector>

namespace poise::runtime {
class NamespaceManager
{
private:
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
        std::vector<Value> structs{};
        std::vector<NamespaceConstant> constants{};
        std::vector<usize> importedNamespaces{};
    };

public:
    // returns wether this is a newly added namespace, or one that's already been compiled.
    // in either case, it will add `namespacePath` to `parent`'s imported namespaces
    [[nodiscard]] auto addNamespace(const std::filesystem::path& namespacePath, std::string namespaceName, std::optional<usize> parent) noexcept -> bool;
    [[nodiscard]] auto namespaceDisplayName(usize namespaceHash) const noexcept -> std::string_view;
    [[nodiscard]] auto namespaceHasImportedNamespace(usize parent, usize imported) const noexcept -> bool;

    auto addFunctionToNamespace(usize namespaceHash, Value function) noexcept -> void;
    [[nodiscard]] auto namespaceFunction(usize namespaceHash, usize functionNameHash) const noexcept -> std::optional<Value>;

    auto addStructToNamespace(usize namespaceHash, Value structure) noexcept -> void;
    [[nodiscard]] auto namespaceStruct(usize namespaceHash, usize structNameHash) const noexcept -> std::optional<Value>;

    auto addConstant(usize namespaceHash, Value value, std::string name, bool isExported) noexcept -> void;
    [[nodiscard]] auto hasConstant(usize namespaceHash, std::string_view constantName) const noexcept -> bool;
    [[nodiscard]] auto getConstant(usize namespaceHash, std::string_view constantName) const noexcept -> std::optional<NamespaceConstant>;

private:
    struct usizeer
    {
        [[nodiscard]] auto operator()(const NamespaceInfo& namespaceInfo) const -> usize
        {
            return std::hash<std::filesystem::path>{}(namespaceInfo.path);
        }
    };

    utils::DualIndexSet<NamespaceInfo, usizeer> m_namespaceInfoLookup;
};
}   // namespace poise::runtime

#endif  // #ifndef POISE_NAMESPACE_MANAGER_HPP

