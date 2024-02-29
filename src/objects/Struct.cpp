#include "Struct.hpp"

#include <fmt/format.h>

namespace poise::objects {
Struct::Struct(std::string name, bool exported, std::vector<MemberVariable> memberVariables)
    : m_name{std::move(name)}
    , m_nameHash{std::hash<std::string>{}(m_name)}
    , m_exported{exported}
    , m_memberVariables{std::move(memberVariables)}
{

}

auto Struct::asStruct() noexcept -> Struct*
{
    return this;
}

auto Struct::toString() const noexcept -> std::string
{
    return fmt::format("<struct {}>", m_name, fmt::ptr(this));
}

auto Struct::type() const noexcept -> runtime::types::Type
{
    return runtime::types::Type::Struct;
}

auto Struct::findObjectMembers(std::unordered_set<Object*>& objects) const noexcept -> void
{
    for (const auto& member : m_memberVariables) {
        if (auto object = member.value.object()) {
            if (const auto [_, inserted] = objects.insert(object); inserted) {
                object->findObjectMembers(objects);
            }
        }
    }
}

auto Struct::removeObjectMembers() noexcept -> void
{
    for (auto& member : m_memberVariables) {
        if (member.value.object() != nullptr) {
            member.value = runtime::Value::none();
        }
    }
}

auto Struct::anyMemberMatchesRecursive(const Object* object) const noexcept -> bool
{
    return std::ranges::any_of(m_memberVariables, [object, this] (const auto& member) -> bool {
        const auto memberObj = member.value.object();
        return memberObj != nullptr && (memberObj == this || memberObj == object || memberObj->anyMemberMatchesRecursive(object));
    });
}

auto Struct::findMember(std::string_view memberName) const -> std::optional<runtime::Value>
{
    return findMember(std::hash<std::string_view>{}(memberName));
}

auto Struct::findMember(usize memberNameHash) const -> std::optional<runtime::Value>
{
    for (const auto& member : m_memberVariables) {
        if (member.nameHash == memberNameHash) {
            return member.value;
        }
    }

    return {};
}

auto Struct::assignMember(std::string_view memberName, runtime::Value value) -> bool
{
    return assignMember(std::hash<std::string_view>{}(memberName), std::move(value));
}

auto Struct::assignMember(usize memberNameHash, runtime::Value value) -> bool
{
    for (auto& member : m_memberVariables) {
        if (member.nameHash == memberNameHash) {
            member.value = std::move(value);
            return true;
        }
    }

    return false;
}

auto Struct::exported() const noexcept -> bool
{
    return m_exported;
}

auto Struct::name() const noexcept -> std::string_view
{
    return m_name;
}

auto Struct::nameHash() const noexcept -> usize
{
    return m_nameHash;
}
} // namespace poise::objects

