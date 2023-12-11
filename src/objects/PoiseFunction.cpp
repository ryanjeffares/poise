#include "PoiseFunction.hpp"
#include "PoiseType.hpp"

#include <fmt/core.h>
#include <fmt/format.h>

#include <ranges>

namespace poise::objects {
static std::hash<std::string> s_hasher;

PoiseFunction::PoiseFunction(std::string name, std::filesystem::path filePath, u8 arity, bool isExported)
    : m_name{std::move(name)}
    , m_filePath{std::move(filePath)}
    , m_arity{arity}
    , m_nameHash{s_hasher(m_name)}
    , m_isExported{isExported}
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

auto PoiseFunction::print() const -> void
{
    fmt::print("{}", toString());
}

auto PoiseFunction::printLn() const -> void
{
    fmt::print("{}\n", toString());
}

auto PoiseFunction::toString() const noexcept -> std::string
{
    return fmt::format("<function instance '{}' at {}>", m_name, fmt::ptr(this));
}

auto PoiseFunction::typeValue() const noexcept -> const runtime::Value&
{
    return types::s_functionType;
}

auto PoiseFunction::objectType() const noexcept -> ObjectType
{
    return ObjectType::Function;
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

auto PoiseFunction::exported() const noexcept -> bool
{
    return m_isExported;
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
    printLn();

    fmt::print("Ops:\n");
    for (auto i = 0_uz; i < m_ops.size(); i++) {
        fmt::print("\t{}: {} at line {}\n", i, m_ops[i].op, m_ops[i].line);
    }

    fmt::print("Constants:\n");
    for (auto i = 0_uz; i < m_constants.size(); i++) {
        fmt::print("\t{}: {}\n", i, m_constants[i]);
    }
}
}   // namespace poise::objects
