#ifndef POISE_TYPE_HPP
#define POISE_TYPE_HPP

#include "PoiseObject.hpp"
#include "../runtime/Types.hpp"
#include "../runtime/Value.hpp"

#include <optional>
#include <span>
#include <unordered_map>

namespace poise::objects {
class PoiseType : public PoiseObject
{
public:
    PoiseType(runtime::types::Type type, std::string name, runtime::Value constructorFunction = runtime::Value::none());
    ~PoiseType() override = default;

    [[nodiscard]] auto toString() const noexcept -> std::string override;
    [[nodiscard]] auto type() const noexcept -> runtime::types::Type override;
    [[nodiscard]] auto typeValue() const noexcept -> const runtime::Value& override;

    [[nodiscard]] auto asType() noexcept -> PoiseType* override;

    [[nodiscard]] auto heldType() const noexcept -> runtime::types::Type;
    [[nodiscard]] auto typeName() const noexcept -> std::string_view;
    [[nodiscard]] auto isPrimitiveType() const noexcept -> bool;
    [[nodiscard]] auto hasConstructor() const noexcept -> bool;

    [[nodiscard]] auto construct(std::span<const runtime::Value>) const -> runtime::Value;

    auto addExtensionFunction(runtime::Value extensionFunction) -> void;
    [[nodiscard]] auto findExtensionFunction(usize functionNameHash) const -> std::optional<runtime::Value>;
    [[nodiscard]] auto findExtensionFunction(std::string_view functionName) const -> std::optional<runtime::Value>;

private:
    runtime::types::Type m_type;
    std::string m_typeName;
    runtime::Value m_constructorFunction;
    std::vector<runtime::Value> m_extensionFunctions;
};  // class PoiseType
}   // namespace poise::objects

#endif  // #ifndef POISE_TYPE_HPP
