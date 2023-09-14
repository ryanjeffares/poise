#include "Vm.hpp"

namespace poise::runtime
{
    auto Vm::setCurrentFunction(objects::PoiseFunction* function) -> void
    {
        m_currentFunction = function;
    }

    auto Vm::emitOp(Op op) -> void
    {
        if (m_currentFunction == nullptr) {
            m_globalOps.push_back(op);
        } else {
            m_currentFunction->emitOp(op);
        }
    }

    auto Vm::emitConstant(Value value) -> void
    {
        if (m_currentFunction == nullptr) {
            m_globalConstants.emplace_back(std::move(value));
        } else {
            m_currentFunction->emitConstant(std::move(value));
        }
    }

    auto Vm::run() -> RunResult
    {

        return RunResult::Success;
    }
}
