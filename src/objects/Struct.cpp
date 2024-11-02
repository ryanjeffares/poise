#include "Struct.hpp"

#include <algorithm>

#include <boost/functional.hpp>
#include <fmt/format.h>

namespace poise::objects {
Struct::Struct(std::string name, bool exported, std::vector<MemberVariable> memberVariables)
    : m_exported{exported}
    , m_name{std::move(name)}
    , m_nameHash{boost::hash<std::string>{}(m_name)}
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

auto Struct::findObjectMembers(boost::unordered_flat_set<Object*>& objects) const noexcept -> void
{
    for (const auto& member : m_memberVariables) {
        if (auto object = member.defaultValue.object()) {
            if (const auto [_, inserted] = objects.insert(object); inserted) {
                object->findObjectMembers(objects);
            }
        }
    }
}

auto Struct::removeObjectMembers() noexcept -> void
{
    for (auto& member : m_memberVariables) {
        if (member.defaultValue.object() != nullptr) {
            member.defaultValue = runtime::Value::none();
        }
    }
}

auto Struct::anyMemberMatchesRecursive(const Object* object) const noexcept -> bool
{
    return std::ranges::any_of(m_memberVariables, [object, this] (const auto& member) -> bool {
        const auto memberObj = member.defaultValue.object();
        return memberObj != nullptr && (memberObj == this || memberObj == object || memberObj->anyMemberMatchesRecursive(object));
    });
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

auto Struct::memberVariables() const noexcept -> std::span<const MemberVariable>
{
    return m_memberVariables;
}

auto Struct::hasMember(usize memberNameHash) const noexcept -> bool
{
    return std::ranges::any_of(m_memberVariables, [memberNameHash] (const auto& memberVariable) -> bool {
        return memberVariable.nameHash == memberNameHash;
    });
}
} // namespace poise::objects

