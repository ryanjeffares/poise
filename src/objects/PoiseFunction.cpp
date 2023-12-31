#include "PoiseFunction.hpp"

#include <fmt/core.h>
#include <fmt/format.h>

namespace poise::objects {
static std::hash<std::string> s_hasher;

PoiseFunction::PoiseFunction(std::string name, std::filesystem::path filePath, usize namespaceHash, u8 arity, bool isExported, bool hasPack)
    : m_name{std::move(name)}
    , m_filePath{std::move(filePath)}
    , m_arity{arity}
    , m_nameHash{s_hasher(m_name)}
    , m_namespaceHash{namespaceHash}
    , m_isExported{isExported}
    , m_hasVariadicParams{hasPack}
{

}

auto PoiseFunction::asFunction() noexcept -> PoiseFunction*
{
    return this;
}

auto PoiseFunction::emitOp(runtime::Op op, usize line) noexcept -> void
{
    m_ops.push_back({op, line});
}

auto PoiseFunction::emitConstant(runtime::Value value) noexcept -> void
{
    m_constants.emplace_back(std::move(value));
}

auto PoiseFunction::setConstant(runtime::Value value, usize index) noexcept -> void
{
    m_constants[index] = std::move(value);
}

auto PoiseFunction::toString() const noexcept -> std::string
{
    return fmt::format("<function instance '{}' at {}>", m_name, fmt::ptr(this));
}

auto PoiseFunction::type() const noexcept -> runtime::types::Type
{
    return runtime::types::Type::Function;
}

auto PoiseFunction::opList() const noexcept -> std::span<const runtime::OpLine>
{
    return m_ops;
}

auto PoiseFunction::numOps() const noexcept -> usize
{
    return m_ops.size();
}

auto PoiseFunction::constantList() const noexcept -> std::span<const runtime::Value>
{
    return m_constants;
}

auto PoiseFunction::numConstants() const noexcept -> usize
{
    return m_constants.size();
}

auto PoiseFunction::name() const noexcept -> std::string_view
{
    return m_name;
}

auto PoiseFunction::filePath() const noexcept -> const std::filesystem::path&
{
    return m_filePath;
}

auto PoiseFunction::arity() const noexcept -> u8
{
    return m_arity;
}

auto PoiseFunction::nameHash() const noexcept -> usize
{
    return m_nameHash;
}

auto PoiseFunction::namespaceHash() const noexcept -> usize
{
    return m_namespaceHash;
}

auto PoiseFunction::exported() const noexcept -> bool
{
    return m_isExported;
}

auto PoiseFunction::hasVariadicParams() const noexcept -> bool
{
    return m_hasVariadicParams;
}

auto PoiseFunction::numLambdas() const noexcept -> u32
{
    return m_numLambdas;
}

auto PoiseFunction::lamdaAdded() noexcept -> void
{
    m_numLambdas++;
}

auto PoiseFunction::addCapture(runtime::Value value) noexcept -> void
{
    m_captures.emplace_back(std::move(value));
}

auto PoiseFunction::getCapture(poise::usize index) const noexcept -> const runtime::Value&
{
    return m_captures[index];
}

auto PoiseFunction::printOps() const -> void
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

auto PoiseFunction::shallowClone() const noexcept -> runtime::Value
{
    auto value = runtime::Value::createObject<PoiseFunction>(m_name, m_filePath, m_namespaceHash, m_arity, m_isExported, m_hasVariadicParams);
    auto function = value.object()->asFunction();
    function->copyData(*this);
    return value;
}

auto PoiseFunction::copyData(const PoiseFunction& other) -> void
{
    m_numLambdas = other.numLambdas();
    m_ops = other.m_ops;
    m_constants = other.m_constants;
}
}   // namespace poise::objects
