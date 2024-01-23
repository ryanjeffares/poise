#include "Function.hpp"

#include <fmt/core.h>
#include <fmt/format.h>

#include <algorithm>

namespace poise::objects {
static std::hash<std::string> s_hasher;

Function::Function(std::string name, std::filesystem::path filePath, usize namespaceHash, u8 arity, bool isExported, bool hasPack)
    : m_name{std::move(name)}
    , m_filePath{std::move(filePath)}
    , m_arity{arity}
    , m_nameHash{s_hasher(m_name)}
    , m_namespaceHash{namespaceHash}
    , m_isExported{isExported}
    , m_hasVariadicParams{hasPack}
{

}

auto Function::asFunction() noexcept -> Function*
{
    return this;
}

auto Function::emitOp(runtime::Op op, usize line) noexcept -> void
{
    m_ops.push_back({op, line});
}

auto Function::emitConstant(runtime::Value value) noexcept -> void
{
    m_constants.emplace_back(std::move(value));
}

auto Function::setConstant(runtime::Value value, usize index) noexcept -> void
{
    m_constants[index] = std::move(value);
}

auto Function::toString() const noexcept -> std::string
{
    return fmt::format("<function instance '{}' at {}>", m_name, fmt::ptr(this));
}

auto Function::type() const noexcept -> runtime::types::Type
{
    return runtime::types::Type::Function;
}

auto Function::findObjectMembers(std::vector<Object*>& objects) const noexcept -> void
{
    for (const auto& capture : m_captures) {
        if (const auto object = capture.object()) {
            if (!std::ranges::contains(objects, object)) {
                objects.push_back(object);
                object->findObjectMembers(objects);
            }
        }
    }
}

auto Function::removeObjectMembers() noexcept -> void
{
    for (auto& capture : m_captures) {
        if (capture.object() != nullptr) {
            capture = runtime::Value::none();
        }
    }
}

auto Function::anyMemberMatchesRecursive(const Object* object) const noexcept -> bool
{
    return std::ranges::any_of(m_captures, [object, this] (const auto& capture) -> bool {
        const auto member = capture.object();
        return member != nullptr && (member == this || member == object || member->anyMemberMatchesRecursive(object));
    });
}

auto Function::opList() const noexcept -> std::span<const runtime::OpLine>
{
    return m_ops;
}

auto Function::numOps() const noexcept -> usize
{
    return m_ops.size();
}

auto Function::constantList() const noexcept -> std::span<const runtime::Value>
{
    return m_constants;
}

auto Function::numConstants() const noexcept -> usize
{
    return m_constants.size();
}

auto Function::name() const noexcept -> std::string_view
{
    return m_name;
}

auto Function::filePath() const noexcept -> const std::filesystem::path&
{
    return m_filePath;
}

auto Function::arity() const noexcept -> u8
{
    return m_arity;
}

auto Function::nameHash() const noexcept -> usize
{
    return m_nameHash;
}

auto Function::namespaceHash() const noexcept -> usize
{
    return m_namespaceHash;
}

auto Function::exported() const noexcept -> bool
{
    return m_isExported;
}

auto Function::hasVariadicParams() const noexcept -> bool
{
    return m_hasVariadicParams;
}

auto Function::numLambdas() const noexcept -> u32
{
    return m_numLambdas;
}

auto Function::lamdaAdded() noexcept -> void
{
    m_numLambdas++;
}

auto Function::addCapture(runtime::Value value) noexcept -> void
{
    m_captures.emplace_back(std::move(value));
}

auto Function::getCapture(usize index) const noexcept -> const runtime::Value&
{
    return m_captures[index];
}

auto Function::printOps() const -> void
{
    fmt::print("{}\n", toString());

    fmt::print("Ops:\n");
    for (auto i = 0_uz; i < m_ops.size(); i++) {
        fmt::print("\t{}: {} at line {}\n", i, m_ops[i].op, m_ops[i].line);
    }

    fmt::print("Constants:\n");
    for (auto i = 0_uz; i < m_constants.size(); i++) {
        fmt::print("\t{}: {}\n", i, m_constants[i]);
    }
}

auto Function::shallowClone() const noexcept -> runtime::Value
{
    auto value = runtime::Value::createObject<Function>(m_name, m_filePath, m_namespaceHash, m_arity, m_isExported, m_hasVariadicParams);
    const auto function = value.object()->asFunction();
    function->copyData(*this);
    return value;
}

auto Function::copyData(const Function& other) -> void
{
    m_numLambdas = other.numLambdas();
    m_ops = other.m_ops;
    m_constants = other.m_constants;
}
}   // namespace poise::objects
