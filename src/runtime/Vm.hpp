#ifndef POISE_VM_HPP
#define POISE_VM_HPP

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
        auto emitOp(Op op) -> void;
        auto emitConstant(Value value) -> void;

        auto run() -> RunResult;

    private:
        std::vector<Op> m_globalOps;
        std::vector<Value> m_globalConstants;

        objects::PoiseFunction* m_currentFunction{nullptr};
    };
}

#endif
