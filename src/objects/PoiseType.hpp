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
    Bool, Float, Int, None, String, Exception, Function, Type,
};
}   // namespace poise::types

namespace poise::objects {
class PoiseType : public PoiseObject
{
public:
    PoiseType(types::Type type, std::string name, runtime::Value constructorFunction = runtime::Value::none());
    ~PoiseType() override = default;

    auto print() const -> void override;
    auto printLn() const -> void override;
    [[nodiscard]] auto toString() const noexcept -> std::string override;
    [[nodiscard]] auto typeValue() const noexcept -> const runtime::Value& override;
    [[nodiscard]] auto objectType() const noexcept -> ObjectType override;

    [[nodiscard]] auto asType() noexcept -> PoiseType* override;

    [[nodiscard]] auto type() const noexcept -> types::Type;
    [[nodiscard]] auto typeName() const noexcept -> std::string_view;
    [[nodiscard]] auto isPrimitiveType() const noexcept -> bool;
    [[nodiscard]] auto hasConstructor() const noexcept -> bool;

    [[nodiscard]] auto construct(std::span<const runtime::Value>) const -> runtime::Value;

private:
    types::Type m_type;
    std::string m_typeName;
    runtime::Value m_constructorFunction;
};  // class PoiseType
}   // namespace poise::objects

namespace poise::types {
inline const runtime::Value s_boolType = runtime::Value::createObject<objects::PoiseType>(types::Type::Bool, "Bool");
inline const runtime::Value s_floatType = runtime::Value::createObject<objects::PoiseType>(types::Type::Float, "Float");
inline const runtime::Value s_intType = runtime::Value::createObject<objects::PoiseType>(types::Type::Int, "Int");
inline const runtime::Value s_noneType = runtime::Value::createObject<objects::PoiseType>(types::Type::None, "None");
inline const runtime::Value s_stringType = runtime::Value::createObject<objects::PoiseType>(types::Type::String, "String");
inline const runtime::Value s_exceptionType = runtime::Value::createObject<objects::PoiseType>(types::Type::Exception, "Exception");
inline const runtime::Value s_functionType = runtime::Value::createObject<objects::PoiseType>(types::Type::Function, "Function");
inline const runtime::Value s_typeType = runtime::Value::createObject<objects::PoiseType>(types::Type::Type, "Type");

inline const std::unordered_map<types::Type, runtime::Value> s_typeLookup = {
    {types::Type::Bool,     s_boolType},
    {types::Type::Float,    s_floatType},
    {types::Type::Int,      s_intType},
    {types::Type::None,     s_noneType},
    {types::Type::String,   s_stringType},
    {types::Type::Exception, s_exceptionType},
    {types::Type::Function, s_functionType},
    {types::Type::Type,     s_typeType},
};
}   // namespace poise::types

#endif  // #ifndef POISE_TYPE_HPP