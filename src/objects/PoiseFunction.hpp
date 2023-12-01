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
        [[nodiscard]] auto typeValue() const -> const runtime::Value& override;
        [[nodiscard]] auto objectType() const -> ObjectType override;

        [[nodiscard]] auto opList() const -> std::span<const runtime::OpLine>;
        [[nodiscard]] auto numOps() const -> usize;
        [[nodiscard]] auto constantList() const -> std::span<const runtime::Value>;
        [[nodiscard]] auto numConstants() const -> usize;

        [[nodiscard]] auto name() const -> std::string_view;
        [[nodiscard]] auto arity() const -> u8;
        [[nodiscard]] auto numLambdas() const -> u32;
        auto lamdaAdded() -> void;
        auto addCapture(runtime::Value value) -> void;

        auto printOps() const -> void;

    private:
        std::string m_name;
        u8 m_arity;
        u32 m_numLambdas{0};

        std::vector<runtime::OpLine> m_ops;
        std::vector<runtime::Value> m_constants;
        std::vector<runtime::Value> m_captures;
    };
}

#endif
