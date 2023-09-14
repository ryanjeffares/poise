#include "PoiseFunction.hpp"

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

    auto PoiseFunction::emitOp(runtime::Op op) -> void
    {
        m_ops.push_back(op);
    }

    auto PoiseFunction::emitConstant(runtime::Value value) -> void
    {
        m_constants.emplace_back(std::move(value));
    }
}
