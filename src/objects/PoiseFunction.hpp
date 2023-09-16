#ifndef POISE_FUNCTION_HPP
#define POISE_FUNCTION_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "PoiseObject.hpp"
#include "../runtime/Op.hpp"
#include "../runtime/Value.hpp"

namespace poise::objects
{
    class PoiseFunction : public PoiseObject
    {
    public:
        PoiseFunction(std::string name, std::uint8_t arity);
        ~PoiseFunction() override = default;

        auto asFunction() -> PoiseFunction* override;

        auto emitOp(runtime::Op op, std::size_t line) -> void;
        auto emitConstant(runtime::Value value) -> void;

        auto print() const -> void override;
        auto printLn() const -> void override;

    private:
        std::string m_name;
        std::uint8_t m_arity;

        std::vector<runtime::OpLine> m_ops;
        std::vector<runtime::Value> m_constants;
    };
}

#endif
