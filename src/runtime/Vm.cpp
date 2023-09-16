#include "Vm.hpp"

#include <fmt/color.h>
#include <fmt/core.h>

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
        std::vector<Value> availabelFunctions;

        std::vector<const std::vector<OpLine>*> opListStack{&m_globalOps};
        std::vector<const std::vector<Value>*> constantListStack{&m_globalConstants};

        std::vector<std::size_t> opIndexStack = {0zu};
        std::vector<std::size_t> constantIndexStack = {0zu};

        auto pop = [&] -> Value {
            auto value = std::move(stack.back());
            stack.pop_back();
            return value;
        };

#define PANIC(message) \
        do { \
            fmt::print(stderr, fmt::emphasis::bold | fmt::fg(fmt::color::red), "PANIC: "); \
            fmt::print(stderr, "{}\n", message); \
            return RunResult::RuntimeError; \
        } while (false) \

        while (true) {
            auto opList = opListStack.back();
            auto constantList = constantListStack.back();
            auto& opIndex = opIndexStack.back();
            auto& constantIndex = constantIndexStack.back();

            auto [op, line] = opList->at(opIndex++);

            switch (op) {
                case Op::Call: {
                    auto value = pop();
                    if (auto object = value.object()) {
                        if (auto function = object->asFunction()) {
                            opListStack.push_back(function->opList());
                            constantListStack.push_back(function->constantList());
                            opIndexStack.push_back(0zu);
                            constantListStack.push_back(0zu);
                        } else {
                            PANIC(fmt::format("{} is not a function", object->toString()));
                        } 
                    } else {
                        PANIC(fmt::format("{} is not callable", value));
                    }
                    break;
                }
                case Op::DeclareFunction: {
                    auto function = constantList->at(constantIndex++);
                    availabelFunctions.emplace_back(std::move(function));
                    break;
                }
                case Op::LoadConstant: {
                    stack.push_back(constantList->at(constantIndex++));
                    break;
                }
                case Op::PrintLn: {
                    auto value = pop();
                    value.printLn();
                    break;
                }
                case Op::Return: {
                    opListStack.pop_back();
                    constantListStack.pop_back();
                    opIndexStack.pop_back();
                    constantIndexStack.pop_back();
                    break;
                }
            }
        }

        return RunResult::Success;
    }
}
