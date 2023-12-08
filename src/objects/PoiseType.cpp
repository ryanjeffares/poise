#include "PoiseType.hpp"
#include "PoiseException.hpp"

#include <fmt/core.h>
#include <fmt/format.h>

namespace poise::objects {
PoiseType::PoiseType(types::Type type, std::string name, runtime::Value constructorFunction)
    : m_type{type}
    , m_typeName{std::move(name)}
    , m_constructorFunction{std::move(constructorFunction)}
{

}

auto PoiseType::print() const -> void
{
    fmt::print("{}", toString());
}

auto PoiseType::printLn() const -> void
{
    fmt::print("{}\n", toString());
}

auto PoiseType::toString() const noexcept -> std::string
{
    return fmt::format("<type instance '{}' at {}>", m_typeName, fmt::ptr(this));
}

auto PoiseType::typeValue() const noexcept -> const runtime::Value&
{
    return types::s_typeType;
}

auto PoiseType::objectType() const noexcept -> ObjectType
{
    return ObjectType::Type;
}

auto PoiseType::asType() noexcept -> PoiseType*
{
    return this;
}

auto PoiseType::type() const noexcept -> types::Type
{
    return m_type;
}

auto PoiseType::typeName() const noexcept -> std::string_view
{
    return m_typeName;
}

auto PoiseType::isPrimitiveType() const noexcept -> bool
{
    return type() == types::Type::Bool ||
           type() == types::Type::Float ||
           type() == types::Type::Int ||
           type() == types::Type::None ||
           type() == types::Type::String;
}

auto PoiseType::hasConstructor() const noexcept -> bool
{
    return m_constructorFunction.type() != runtime::Value::Type::None;
}

auto PoiseType::construct(std::span<const runtime::Value> args) const -> runtime::Value
{
    switch (type()) {
        case types::Type::Bool:
            return !args.empty() && args[0].toBool();
        case types::Type::Float:
            return args.empty() ? 0.0 : args[0].toFloat();
        case types::Type::Int:
            return args.empty() ? 0 : args[0].toInt();
        case types::Type::None:
            return args.empty() || args[0].type() == runtime::Value::Type::None
                   ? runtime::Value::none()
                   : throw PoiseException(PoiseException::ExceptionType::InvalidType, fmt::format("Cannot construct None from '{}'", args[0].type()));
        case types::Type::String:
            return args.empty() ? "" : args[0].toString();
        case types::Type::Exception: {
            if (args.empty()) {
                throw PoiseException(PoiseException::ExceptionType::IncorrectArgCount, "'Function' constructor takes 1 argument but was given none");
            }

            return runtime::Value::createObject<PoiseException>(args[0].toString());
        }
        case types::Type::Function: {
            if (args.empty()) {
                throw PoiseException(PoiseException::ExceptionType::IncorrectArgCount, "'Function' constructor takes 1 argument but was given none");
            }

            if (const auto object = args[0].object()) {
                if (const auto function = object->asFunction()) {
                    return args[0];
                } else {
                    throw PoiseException(PoiseException::ExceptionType::InvalidType, "'Function' can only be constructed from Function or Lambda");
                }
            } else {
                throw PoiseException(PoiseException::ExceptionType::InvalidType, "'Function' can only be constructed from Function or Lambda");
            }
        }
        case types::Type::Type:
            throw PoiseException(PoiseException::ExceptionType::InvalidType, "Cannot construct Type");
        default:
            POISE_UNREACHABLE();
            return runtime::Value::none();
    }
}
}   // namespace poise::objects