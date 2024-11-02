#ifndef POISE_STRUCT_INSTANCE_HPP
#define POISE_STRUCT_INSTANCE_HPP

#include "../Poise.hpp"

#include "Object.hpp"
#include "../runtime/Value.hpp"
#include "../utils/DualIndexSet.hpp"

namespace poise::objects {
class StructInstance : public Object
{
public:
    explicit StructInstance(runtime::Value structType);

    [[nodiscard]] auto structType() const noexcept -> const runtime::Value&;
    [[nodiscard]] auto findMember(usize memberNameHash) const -> const runtime::Value&;
    auto assignMember(usize memberNameHash, runtime::Value value) -> void;
    auto assignMissingMembers() noexcept -> void;

    [[nodiscard]] auto asStructInstance() noexcept -> StructInstance* override;

    [[nodiscard]] auto toString() const noexcept -> std::string override;
    [[nodiscard]] auto type() const noexcept -> runtime::types::Type override;
    auto findObjectMembers(boost::unordered_flat_set<Object*>& objects) const noexcept -> void override;
    auto removeObjectMembers() noexcept -> void override;
    [[nodiscard]] auto anyMemberMatchesRecursive(const Object* object) const noexcept -> bool override;

private:
    struct MemberVariable
    {
        usize nameHash;
        runtime::Value value;
    };

    struct MemberVariableHash
    {
        [[nodiscard]] auto operator()(const MemberVariable& mv) const noexcept -> usize {
            return mv.nameHash;
        }
    };

    runtime::Value m_structType;
    utils::DualIndexSet<MemberVariable, MemberVariableHash> m_memberVariables;
};
} // namespace poise::objects

#endif // #ifndef POISE_STRUCT_INSTANCE_HPP

