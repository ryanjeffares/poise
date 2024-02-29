#ifndef POISE_STRUCT_HPP
#define POISE_STRUCT_HPP

#include "../Poise.hpp"

#include "Object.hpp"
#include "../runtime/Value.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace poise::objects {
class Struct : public Object
{
public:
    struct MemberVariable
    {
        std::string name{};
        usize nameHash{};
        runtime::Value value{};
    };

    explicit Struct(std::string name, bool exported, std::vector<MemberVariable> memberVariables);

    ~Struct() override = default;

    [[nodiscard]] auto asStruct() noexcept -> Struct* override;

    [[nodiscard]] auto toString() const noexcept -> std::string override;
    [[nodiscard]] auto type() const noexcept -> runtime::types::Type override;
    auto findObjectMembers(std::unordered_set<Object*>& objects) const noexcept -> void override;
    auto removeObjectMembers() noexcept -> void override;
    [[nodiscard]] auto anyMemberMatchesRecursive(const Object* object) const noexcept -> bool override;

    [[nodiscard]] auto findMember(std::string_view memberName) const -> std::optional<runtime::Value>;
    [[nodiscard]] auto findMember(usize memberNameHash) const -> std::optional<runtime::Value>;
    [[nodiscard]] auto assignMember(std::string_view memberName, runtime::Value value) -> bool;
    [[nodiscard]] auto assignMember(usize memberNameHash, runtime::Value value) -> bool;

    [[nodiscard]] auto exported() const noexcept -> bool;
    [[nodiscard]] auto name() const noexcept -> std::string_view;
    [[nodiscard]] auto nameHash() const noexcept -> usize;

private:
    std::string m_name;
    usize m_nameHash;
    bool m_exported;
    std::vector<MemberVariable> m_memberVariables;
};
} // namespace poise::objects

#endif // #ifndef POISE_STRUCT_HPP

