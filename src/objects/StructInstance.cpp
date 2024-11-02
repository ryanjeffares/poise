#include "StructInstance.hpp"
#include "Exception.hpp"
#include "../runtime/memory/StringInterner.hpp"
#include "Struct.hpp"

#include <algorithm>

namespace poise::objects
{
StructInstance::StructInstance(runtime::Value structType) : m_structType{std::move(structType)}
{

}

auto StructInstance::structType() const noexcept -> const runtime::Value&
{
    return m_structType;
}

auto StructInstance::findMember(usize memberNameHash) const -> const runtime::Value&
{
    const auto it = m_memberVariables.find(memberNameHash);

    if (it == m_memberVariables.end()) {
        throw Exception{
            Exception::ExceptionType::MemberNotFound,
            fmt::format(
                "Struct {} has no member '{}'",
                m_structType.object()->asStruct()->name(),
                runtime::memory::findInternedString(memberNameHash)
            ),
        };
    }

    return it->second.value;
}

auto StructInstance::assignMember(usize memberNameHash, runtime::Value value) -> void
{
    const auto structType = m_structType.object()->asStruct();

    if (!structType->hasMember(memberNameHash)) {
        throw Exception{
            Exception::ExceptionType::MemberNotFound,
            fmt::format(
                "Struct {} has no member '{}'",
                m_structType.object()->asStruct()->name(),
                runtime::memory::findInternedString(memberNameHash)
            ),
        };
    }

    m_memberVariables.insert(MemberVariable{
        .nameHash = memberNameHash,
        .value = std::move(value),
    });
}

auto StructInstance::assignMissingMembers() noexcept -> void
{
    const auto memberVariables = m_structType.object()->asStruct()->memberVariables();
    for (const auto& [_, nameHash, value] : memberVariables) {
        if (!m_memberVariables.contains(nameHash)) {
            m_memberVariables.insert(MemberVariable{
                .nameHash = nameHash,
                .value = value,
            });
        }
    }
}

auto StructInstance::asStructInstance() noexcept -> StructInstance*
{
    return this;
}

auto StructInstance::toString() const noexcept -> std::string
{
    const auto structType = m_structType.object()->asStruct();
    auto str = fmt::format("{} {{", structType->name());

    const auto memberVariables = structType->memberVariables();
    for (auto i = 0_uz; i < memberVariables.size(); i++) {
        const auto& [name, nameHash, _] = memberVariables[i];
        const auto& value = m_memberVariables.find(nameHash)->second.value;

        if (value.type() == runtime::types::Type::String) {
            str.append(fmt::format("{}='{}'", name, value));
        } else {
            str.append(fmt::format("{}={}", name, value));
        }

        if (i == memberVariables.size() - 1) {
            str.append("}");
        } else {
            str.append(", ");
        }
    }

    return str;
}

auto StructInstance::type() const noexcept -> runtime::types::Type
{
    return runtime::types::Type::StructInstance;
}

auto StructInstance::findObjectMembers(boost::unordered_flat_set<Object*>& objects) const noexcept -> void
{
    const auto structType = m_structType.object()->asStruct();

    for (const auto memberVariables = structType->memberVariables(); const auto& memberVariable : memberVariables) {
        const auto& value = m_memberVariables.find(memberVariable.nameHash)->second.value;
        if (const auto object = value.object()) {
            if (const auto [_, inserted] = objects.insert(object); inserted) {
                object->findObjectMembers(objects);
            }
        }
    }
}

auto StructInstance::removeObjectMembers() noexcept -> void
{
    const auto structType = m_structType.object()->asStruct();

    for (const auto memberVariables = structType->memberVariables(); const auto& memberVariable : memberVariables) {
        auto& value = m_memberVariables.find(memberVariable.nameHash)->second.value;
        if (value.object() != nullptr) {
            value = runtime::Value::none();
        }
    }
}

auto StructInstance::anyMemberMatchesRecursive(const Object* object) const noexcept -> bool
{
    const auto memberVariables = m_structType.object()->asStruct()->memberVariables();
    return std::ranges::any_of(memberVariables, [object, this] (const auto& memberVariable) -> bool {
        const auto& value = m_memberVariables.find(memberVariable.nameHash)->second.value;
        const auto member = value.object();
        return member != nullptr && (member == this || member == object || member->anyMemberMatchesRecursive(object));
    });
}
} // namespace poise::objects

