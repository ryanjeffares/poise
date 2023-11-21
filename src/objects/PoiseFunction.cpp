#include "PoiseFunction.hpp"

#include <fmt/core.h>
#include <fmt/format.h>

namespace poise::objects
{
    PoiseFunction::PoiseFunction(std::string name, u8 arity)
        : m_name{std::move(name)}
        , m_arity{arity}
    {

    }

    auto PoiseFunction::asFunction() -> PoiseFunction*
    {
        return this;
    }

    auto PoiseFunction::emitOp(runtime::Op op, usize line) -> void
    {
        m_ops.push_back({op, line});
    }

    auto PoiseFunction::emitConstant(runtime::Value value) -> void
    {
        m_constants.emplace_back(std::move(value));
    }

    auto PoiseFunction::print() const -> void
    {
        fmt::print("{}", toString());
    }

    auto PoiseFunction::printLn() const -> void
    {
        fmt::print("{}\n", toString());
    }

    auto PoiseFunction::toString() const -> std::string
    {
        return fmt::format("<function instance '{}' at {}>", m_name, fmt::ptr(this));
    }

    auto PoiseFunction::callable() const -> bool
    {
        return true;
    }

    auto PoiseFunction::opList() const -> std::span<const runtime::OpLine>
    {
        return m_ops;
    }

    auto PoiseFunction::constantList() const -> std::span<const runtime::Value>
    {
        return m_constants;
    }

    auto PoiseFunction::name() const -> const std::string&
    {
        return m_name;
    }

    auto PoiseFunction::printOps() const -> void
    {
        printLn();

        fmt::print("Ops:\n");
        for (const auto [op, _] : m_ops) {
            fmt::print("\t{}\n", op);
        }

        fmt::print("Constants:\n");
        for (const auto& constant : m_constants) {
            fmt::print("\t{}\n", constant);
        }
    }
}
