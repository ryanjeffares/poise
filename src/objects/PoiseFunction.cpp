#include "PoiseFunction.hpp"

#include <fmt/core.h>
#include <fmt/format.h>

namespace poise::objects
{
    PoiseFunction::PoiseFunction(std::string name, std::uint8_t arity)
        : m_name{std::move(name)}
        , m_arity{arity}
    {

    }

    auto PoiseFunction::asFunction() -> PoiseFunction*
    {
        return this;
    }

    auto PoiseFunction::emitOp(runtime::Op op, std::size_t line) -> void
    {
        m_ops.emplace_back(op, line);
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
        return fmt::format("<function instance '{}' at {}>\n", m_name, fmt::ptr(this));
    }

    auto PoiseFunction::opList() const -> const std::vector<runtime::OpLine>*
    {
        return &m_ops;
    }

    auto PoiseFunction::constantList() const -> const std::vector<runtime::Value>*
    {
        return &m_constants;
    }
}
