#ifndef POISE_TYPE_HPP
#define POISE_TYPE_HPP

#include "Object.hpp"
#include "../runtime/Value.hpp"

#include <optional>
#include <span>

namespace poise::objects {
class Type : public Object
{
public:
    using ConstructorFn = runtime::Value(*)(std::span<runtime::Value>);

    Type(runtime::types::Type type, std::string name, ConstructorFn constructorFunction);
    ~Type() override = default;

    [[nodiscard]] auto toString() const noexcept -> std::string override;
    [[nodiscard]] auto type() const noexcept -> runtime::types::Type override;

    [[nodiscard]] auto asType() noexcept -> Type* override;

    [[nodiscard]] auto heldType() const noexcept -> runtime::types::Type;
    [[nodiscard]] auto typeName() const noexcept -> std::string_view;
    [[nodiscard]] auto isPrimitiveType() const noexcept -> bool;

    [[nodiscard]] auto construct(std::span<runtime::Value>) const -> runtime::Value;

    auto addExtensionFunction(runtime::Value extensionFunction) -> void;
    [[nodiscard]] auto findExtensionFunction(usize functionNameHash) const -> std::optional<runtime::Value>;
    [[nodiscard]] auto findExtensionFunction(std::string_view functionName) const -> std::optional<runtime::Value>;

private:
    runtime::types::Type m_type;
    std::string m_typeName;
    std::vector<runtime::Value> m_extensionFunctions;
    ConstructorFn m_constructorFunction;
};  // class PoiseType
}   // namespace poise::objects

#endif  // #ifndef POISE_TYPE_HPP
