#ifndef POISE_FUNCTION_HPP
#define POISE_FUNCTION_HPP

#include <cstdint>
#include <span>
#include <vector>

#include "../poise.hpp"

#include "PoiseObject.hpp"
#include "../runtime/Op.hpp"
#include "../runtime/Value.hpp"

namespace poise::objects
{
    class PoiseFunction : public PoiseObject
    {
    public:
        PoiseFunction(std::string name, u8 arity);
        ~PoiseFunction() override = default;

        auto asFunction() -> PoiseFunction* override;

        auto emitOp(runtime::Op op, usize line) -> void;
        auto emitConstant(runtime::Value value) -> void;
        auto setConstant(runtime::Value value, usize index) -> void;

        auto print() const -> void override;
        auto printLn() const -> void override;

        [[nodiscard]] auto toString() const -> std::string override;
        [[nodiscard]] auto callable() const -> bool override;

        [[nodiscard]] auto opList() const -> std::span<const runtime::OpLine>;
        [[nodiscard]] auto numOps() const -> usize;
        [[nodiscard]] auto constantList() const -> std::span<const runtime::Value>;
        [[nodiscard]] auto numConstants() const -> usize;

        [[nodiscard]] auto name() const -> std::string_view;
        [[nodiscard]] auto arity() const -> u8;

        auto printOps() const -> void;

    private:
        std::string m_name;
        [[maybe_unused]] u8 m_arity;

        std::vector<runtime::OpLine> m_ops;
        std::vector<runtime::Value> m_constants;
    };
}

#endif
