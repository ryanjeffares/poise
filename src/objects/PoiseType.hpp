#ifndef POISE_TYPE_HPP
#define POISE_TYPE_HPP

#include "PoiseObject.hpp"
#include "../runtime/Value.hpp"

#include <optional>
#include <span>
#include <unordered_map>

namespace poise::types
{
    enum class Type
    {
        // order matches primitive type tokens and Value::Type primitives
        Bool, Float, Int, None, String, Function, Type,
    };
}

namespace poise::objects
{
    class PoiseType : public PoiseObject
    {
    public:
        PoiseType(types::Type type, std::string name, runtime::Value constructorFunction = runtime::Value::none());
        ~PoiseType() override = default;

        auto print() const -> void override;
        auto printLn() const -> void override;
        [[nodiscard]] auto toString() const -> std::string override;
        [[nodiscard]] auto typeValue() const -> const runtime::Value& override;
        [[nodiscard]] auto objectType() const -> ObjectType override;

        [[nodiscard]] auto asType() -> PoiseType* override;

        [[nodiscard]] auto type() const -> types::Type;
        [[nodiscard]] auto typeName() const -> std::string_view;
        [[nodiscard]] auto isPrimitiveType() const -> bool;
        [[nodiscard]] auto hasConstructor() const -> bool;

        [[nodiscard]] auto construct(std::span<const runtime::Value>) const -> runtime::Value;

    private:
        types::Type m_type;
        std::string m_typeName;
        runtime::Value m_constructorFunction;
    };
}

namespace poise::types
{
    inline const runtime::Value s_boolType = runtime::Value::createObject<objects::PoiseType>(types::Type::Bool, "Bool");
    inline const runtime::Value s_floatType = runtime::Value::createObject<objects::PoiseType>(types::Type::Float, "Float");
    inline const runtime::Value s_functionType = runtime::Value::createObject<objects::PoiseType>(types::Type::Function, "Function");
    inline const runtime::Value s_intType = runtime::Value::createObject<objects::PoiseType>(types::Type::Int, "Int");
    inline const runtime::Value s_noneType = runtime::Value::createObject<objects::PoiseType>(types::Type::None, "None");
    inline const runtime::Value s_stringType = runtime::Value::createObject<objects::PoiseType>(types::Type::String, "String");
    inline const runtime::Value s_typeType = runtime::Value::createObject<objects::PoiseType>(types::Type::Type, "Type");

    inline const std::unordered_map<types::Type, runtime::Value> s_typeLookup = {
        {types::Type::Bool, s_boolType},
        {types::Type::Float, s_floatType},
        {types::Type::Function, s_functionType},
        {types::Type::Int, s_intType},
        {types::Type::None, s_noneType},
        {types::Type::String, s_stringType},
        {types::Type::Type, s_typeType},
    };
}

#endif  // #ifndef POISE_TYPE_HPP