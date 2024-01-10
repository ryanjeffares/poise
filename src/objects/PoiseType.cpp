#include "Objects.hpp"

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

namespace poise::objects {
PoiseType::PoiseType(runtime::types::Type type, std::string name, ConstructorFn constructorFunction)
    : m_type{type}
    , m_typeName{std::move(name)}
    , m_constructorFunction{constructorFunction}
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

auto PoiseType::construct(std::span<runtime::Value> args) const -> runtime::Value
{
    POISE_ASSERT(m_constructorFunction != nullptr, fmt::format("Constructor function not assigned for {}", heldType()));
    return m_constructorFunction(args);
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
                fmt::format("Ambiguous extension function call: '{}()' defined in {}", functionName, fmt::join(filePaths.begin(), filePaths.end(), " and ")));
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
