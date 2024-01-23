#include "Objects.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <ranges>

namespace poise::objects {
Type::Type(runtime::types::Type type, std::string name, ConstructorFn constructorFunction)
    : m_type{type}
    , m_typeName{std::move(name)}
    , m_constructorFunction{constructorFunction}
{

}

auto Type::toString() const noexcept -> std::string
{
    return fmt::format("<type instance '{}' at {}>", m_typeName, fmt::ptr(this));
}

auto Type::type() const noexcept -> runtime::types::Type
{
    return runtime::types::Type::Type;
}

auto Type::findObjectMembers([[maybe_unused]] std::vector<Object*>& objects) const noexcept -> void
{

}

auto Type::removeObjectMembers() noexcept -> void
{

}

auto Type::asType() noexcept -> Type*
{
    return this;
}

auto Type::heldType() const noexcept -> runtime::types::Type
{
    return m_type;
}

auto Type::typeName() const noexcept -> std::string_view
{
    return m_typeName;
}

auto Type::isPrimitiveType() const noexcept -> bool
{
    return heldType() == runtime::types::Type::Bool ||
           heldType() == runtime::types::Type::Float ||
           heldType() == runtime::types::Type::Int ||
           heldType() == runtime::types::Type::None ||
           heldType() == runtime::types::Type::String;
}

auto Type::construct(std::span<runtime::Value> args) const -> runtime::Value
{
    POISE_ASSERT(m_constructorFunction != nullptr, fmt::format("Constructor function not assigned for {}", heldType()));
    return m_constructorFunction(args);
}

auto Type::addExtensionFunction(runtime::Value extensionFunction) -> void
{
    m_extensionFunctions.emplace_back(std::move(extensionFunction));
}

auto Type::findExtensionFunction(usize functionNameHash) const -> std::optional<runtime::Value>
{
    const auto count = std::ranges::count_if(m_extensionFunctions, [functionNameHash] (const auto& value) -> bool {
        return value.object()->asFunction()->nameHash() == functionNameHash;
    });

    switch (count) {
        case 0:
            return {};
        case 1:
            return *std::ranges::find_if(m_extensionFunctions, [functionNameHash] (const auto& value) -> bool {
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

            throw Exception(Exception::ExceptionType::AmbiguousCall,
                fmt::format("Ambiguous extension function call: '{}()' defined in {}", functionName, fmt::join(filePaths.begin(), filePaths.end(), " and ")));
        }
    }
}

auto Type::findExtensionFunction(std::string_view functionName) const -> std::optional<runtime::Value>
{
    if (const auto it = std::ranges::find_if(m_extensionFunctions, [functionName] (const auto& value) -> bool {
        return value.object()->asFunction()->name() == functionName;
    }); it != m_extensionFunctions.end()) {
        return *it;
    }

    return {};
}
}   // namespace poise::objects
