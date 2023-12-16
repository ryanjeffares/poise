#include "PoiseType.hpp"
#include "PoiseException.hpp"
#include "PoiseFunction.hpp"
#include "iterables/PoiseList.hpp"

#include <fmt/core.h>
#include <fmt/format.h>

#include <ranges>

namespace poise::objects {
PoiseType::PoiseType(runtime::types::Type type, std::string name, runtime::Value constructorFunction)
    : m_type{type}
    , m_typeName{std::move(name)}
    , m_constructorFunction{std::move(constructorFunction)}
{

}

auto PoiseType::toString() const noexcept -> std::string
{
    return fmt::format("<type instance '{}' at {}>", m_typeName, fmt::ptr(this));
}

auto PoiseType::type() const noexcept -> runtime::types::Type
{
    return runtime::types::Type::Type;
}

auto PoiseType::asType() noexcept -> PoiseType*
{
    return this;
}

auto PoiseType::heldType() const noexcept -> runtime::types::Type
{
    return m_type;
}

auto PoiseType::typeName() const noexcept -> std::string_view
{
    return m_typeName;
}

auto PoiseType::isPrimitiveType() const noexcept -> bool
{
    return heldType() == runtime::types::Type::Bool ||
           heldType() == runtime::types::Type::Float ||
           heldType() == runtime::types::Type::Int ||
           heldType() == runtime::types::Type::None ||
           heldType() == runtime::types::Type::String;
}

auto PoiseType::hasConstructor() const noexcept -> bool
{
    return m_constructorFunction.type() != runtime::types::Type::None;
}

auto PoiseType::construct(std::span<runtime::Value> args) const -> runtime::Value
{
    switch (heldType()) {
        case runtime::types::Type::Bool:
            return !args.empty() && args[0].toBool();
        case runtime::types::Type::Float:
            return args.empty() ? 0.0 : args[0].toFloat();
        case runtime::types::Type::Int:
            return args.empty() ? 0 : args[0].toInt();
        case runtime::types::Type::None:
            return args.empty() || args[0].type() == runtime::types::Type::None
                   ? runtime::Value::none()
                   : throw PoiseException(PoiseException::ExceptionType::InvalidType, fmt::format("Cannot construct None from '{}'", args[0].type()));
        case runtime::types::Type::String:
            return args.empty() ? "" : args[0].toString();
        case runtime::types::Type::Exception: {
            if (args.empty()) {
                throw PoiseException(PoiseException::ExceptionType::IncorrectArgCount, "'Function' constructor takes 1 argument but was given none");
            }

            return runtime::Value::createObject<PoiseException>(args[0].toString());
        }
        case runtime::types::Type::Function: {
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
        case runtime::types::Type::List: {
            return runtime::Value::createObject<iterables::PoiseList>(std::vector<runtime::Value>{
                std::make_move_iterator(args.begin()),
                std::make_move_iterator(args.end())
            });
        }
        case runtime::types::Type::Type:
            throw PoiseException(PoiseException::ExceptionType::InvalidType, "Cannot construct Type");
        default:
            POISE_UNREACHABLE();
            return runtime::Value::none();
    }
}

auto PoiseType::addExtensionFunction(runtime::Value extensionFunction) -> void
{
    m_extensionFunctions.emplace_back(std::move(extensionFunction));
}

auto PoiseType::findExtensionFunction(usize functionNameHash) const -> std::optional<runtime::Value>
{
    const auto count = std::count_if(m_extensionFunctions.cbegin(), m_extensionFunctions.cend(), [functionNameHash] (const runtime::Value& value) {
        return value.object()->asFunction()->nameHash() == functionNameHash;
    });

    switch (count) {
        case 0:
            return {};
        case 1:
            return *std::find_if(m_extensionFunctions.begin(), m_extensionFunctions.end(), [functionNameHash] (const runtime::Value& value) {
                return value.object()->asFunction()->nameHash() == functionNameHash;
            });
        default: {
            std::string_view functionName;
            std::vector<std::string> filePaths;
            for (const auto& value : m_extensionFunctions) {
                if (value.object()->asFunction()->nameHash() == functionNameHash) {
                    functionName = value.object()->asFunction()->name();
                    filePaths.emplace_back(value.object()->asFunction()->filePath().string());
                }
            }

            throw PoiseException(PoiseException::ExceptionType::AmbiguousCall,
                fmt::format("Ambiguous extension function call: '{}()' defined in {}", functionName, fmt::join(filePaths.begin(), filePaths.end(), " and")));
        }
    }
}

auto PoiseType::findExtensionFunction(std::string_view functionName) const -> std::optional<runtime::Value>
{
    if (const auto it = std::find_if(m_extensionFunctions.cbegin(), m_extensionFunctions.cend(), [functionName](const runtime::Value& value) {
        return value.object()->asFunction()->name() == functionName;
    }); it != m_extensionFunctions.cend()) {
        return *it;
    }

    return {};
}
}   // namespace poise::objects