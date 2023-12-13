#ifndef POISE_TYPE_HPP
#define POISE_TYPE_HPP

#include "PoiseObject.hpp"
#include "../runtime/Value.hpp"

#include <optional>
#include <span>
#include <unordered_map>

namespace poise::types {
enum class Type
{
    // order matches primitive type tokens and Value::Type primitives
    Bool, Float, Int, None, String, Exception, Function, Iterator, Type,
};
}   // namespace poise::types

namespace poise::objects {
class PoiseType : public PoiseObject
{
public:
    PoiseType(types::Type type, std::string name, runtime::Value constructorFunction = runtime::Value::none());
    ~PoiseType() override = default;

    [[nodiscard]] auto toString() const noexcept -> std::string override;
    [[nodiscard]] auto typeValue() const noexcept -> const runtime::Value& override;

    [[nodiscard]] auto asType() noexcept -> PoiseType* override;

    [[nodiscard]] auto type() const noexcept -> types::Type;
    [[nodiscard]] auto typeName() const noexcept -> std::string_view;
    [[nodiscard]] auto isPrimitiveType() const noexcept -> bool;
    [[nodiscard]] auto hasConstructor() const noexcept -> bool;

    [[nodiscard]] auto construct(std::span<const runtime::Value>) const -> runtime::Value;

    auto addExtensionFunction(runtime::Value extensionFunction) -> void;
    [[nodiscard]] auto findExtensionFunction(usize functionNameHash) const -> std::optional<runtime::Value>;
    [[nodiscard]] auto findExtensionFunction(std::string_view functionName) const -> std::optional<runtime::Value>;

private:
    types::Type m_type;
    std::string m_typeName;
    runtime::Value m_constructorFunction;
    std::vector<runtime::Value> m_extensionFunctions;
};  // class PoiseType
}   // namespace poise::objects

namespace poise::types {
namespace detail {
inline const runtime::Value s_boolType = runtime::Value::createObject<objects::PoiseType>(Type::Bool, "Bool");
inline const runtime::Value s_floatType = runtime::Value::createObject<objects::PoiseType>(Type::Float, "Float");
inline const runtime::Value s_intType = runtime::Value::createObject<objects::PoiseType>(Type::Int, "Int");
inline const runtime::Value s_noneType = runtime::Value::createObject<objects::PoiseType>(Type::None, "None");
inline const runtime::Value s_stringType = runtime::Value::createObject<objects::PoiseType>(Type::String, "String");
inline const runtime::Value s_exceptionType = runtime::Value::createObject<objects::PoiseType>(Type::Exception, "Exception");
inline const runtime::Value s_functionType = runtime::Value::createObject<objects::PoiseType>(Type::Function, "Function");
inline const runtime::Value s_iteratorType = runtime::Value::createObject<objects::PoiseType>(Type::Iterator, "Iterator");
inline const runtime::Value s_typeType = runtime::Value::createObject<objects::PoiseType>(Type::Type, "Type");

inline const std::unordered_map<Type, runtime::Value> s_typeLookup = {
    {types::Type::Bool, s_boolType},
    {types::Type::Float, s_floatType},
    {types::Type::Int, s_intType},
    {types::Type::None, s_noneType},
    {types::Type::String, s_stringType},
    {types::Type::Exception, s_exceptionType},
    {types::Type::Function, s_functionType},
    {types::Type::Iterator, s_iteratorType},
    {types::Type::Type, s_typeType},
};
}

inline auto associateExtensionFunction(Type type, runtime::Value function) noexcept
{
    detail::s_typeLookup.at(type).object()->asType()->addExtensionFunction(std::move(function));
}

inline auto typeValue(Type type) noexcept -> const runtime::Value&
{
    return detail::s_typeLookup.at(type);
}
}   // namespace poise::types

#endif  // #ifndef POISE_TYPE_HPP