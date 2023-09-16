#include "Vm.hpp"

namespace poise::runtime
{
    auto Vm::setCurrentFunction(objects::PoiseFunction* function) -> void
    {
        m_currentFunction = function;
    }

    auto Vm::getCurrentFunction() -> objects::PoiseFunction*
    {
        return m_currentFunction;
    }

    auto Vm::emitOp(Op op, std::size_t line) -> void
    {
        if (m_currentFunction == nullptr) {
            m_globalOps.emplace_back(op, line);
        } else {
            m_currentFunction->emitOp(op, line);
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
        std::vector<Value> stack;
        std::vector<Value> functions;

        auto currentOpList = &m_globalOps;
        auto currentConstantList = &m_globalConstants;

        auto currentOpIndex = 0zu;
        auto currentConstantIndex = 0zu;

        auto pop = [&] -> Value {
            auto value = std::move(stack.back());
            stack.pop_back();
            return value;
        };

        while (true) {
            auto [op, line] = currentOpList->at(currentOpIndex++);

            switch (op) {
                case Op::Call: {
                    auto function = pop();
                    break;
                }
                case Op::DeclareFunction: {
                    auto function = currentConstantList->at(currentConstantIndex++);
                    functions.emplace_back(std::move(function));
                    break;
                }
                case Op::LoadConstant: {
                    stack.push_back(currentConstantList->at(currentConstantIndex++));
                    break;
                }
                case Op::PrintLn: {
                    auto value = pop();
                    value.printLn();
                    break;
                }
            }
        }

        return RunResult::Success;
    }
}
