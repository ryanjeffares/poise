#ifndef POISE_VM_HPP
#define POISE_VM_HPP

#include "../poise.hpp"

#include "../objects/PoiseFunction.hpp"
#include "Op.hpp"
#include "Value.hpp"

#include <vector>

namespace poise::runtime
{
    class Vm
    {
    public:
        enum class RunResult
        {
            Success,
            InitError,
            RuntimeError,
        };

        auto setCurrentFunction(objects::PoiseFunction* function) -> void;
        auto getCurrentFunction() -> objects::PoiseFunction*;

        auto emitOp(Op op, usize line) -> void;
        auto emitConstant(Value value) -> void;

        auto run() -> RunResult;

    private:
        std::vector<OpLine> m_globalOps;
        std::vector<Value> m_globalConstants;

        objects::PoiseFunction* m_currentFunction{nullptr};
    };
}

#endif
