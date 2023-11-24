#include "Vm.hpp"

#include <fmt/color.h>
#include <fmt/core.h>

namespace poise::runtime
{
    auto Vm::setCurrentFunction(objects::PoiseFunction* function) -> void
    {
        m_currentFunction = function;
    }

    auto Vm::getCurrentFunction() const -> objects::PoiseFunction*
    {
        return m_currentFunction;
    }

    auto Vm::emitOp(Op op, usize line) -> void
    {
        if (m_currentFunction == nullptr) {
            m_globalOps.push_back({op, line});
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
        std::vector<Value> localVariables;
        std::vector<Value> availableFunctions;

        std::vector<std::span<const OpLine>> opListStack{m_globalOps};
        std::vector<std::span<const Value>> constantListStack{m_globalConstants};

        std::vector<usize> localIndexOffsetStack = {0zu};
        std::vector<usize> opIndexStack = {0zu};
        std::vector<usize> constantIndexStack = {0zu};

        auto pop = [&stack] -> Value {
            POISE_ASSERT(!stack.empty(), "Stack is empty, there has been an error in codegen");
            auto value = std::move(stack.back());
            stack.pop_back();
            return value;
        };

        [[maybe_unused]] auto popTwo = [&stack] -> std::tuple<Value, Value> {
            POISE_ASSERT(stack.size() >= 2, "Stack is not big enough, there has been an error in codegen");
            auto value1 = std::move(stack.back());
            stack.pop_back();
            auto value2 = std::move(stack.back());
            stack.pop_back();
            return {std::move(value2), std::move(value1)};
        };

#define PANIC(message)                                                                      \
        do {                                                                                \
            fmt::print(stderr, fmt::emphasis::bold | fmt::fg(fmt::color::red), "PANIC: ");  \
            fmt::print(stderr, "{}\n", message);                                            \
            return RunResult::RuntimeError;                                                 \
        } while (false)                                                                     \

        while (true) {
            const auto opList = opListStack.back();
            const auto constantList = constantListStack.back();
            auto& opIndex = opIndexStack.back();
            auto& constantIndex = constantIndexStack.back();

            const auto [op, line] = opList[opIndex++];

            switch (op) {
                case Op::DeclareFunction: {
                    auto function = constantList[constantIndex++];
                    availableFunctions.emplace_back(std::move(function));
                    break;
                }
                case Op::DeclareLocal: {
                    auto value = pop();
                    localVariables.emplace_back(std::move(value));
                    break;
                }
                case Op::LoadConstant: {
                    stack.push_back(constantList[constantIndex++]);
                    break;
                }
                case Op::LoadFunction: {
                    const auto& functionName = constantList[constantIndex++];
                    const auto it = std::find_if(availableFunctions.begin(), availableFunctions.end(), [&functionName] (const Value& function) {
                        return function.object()->asFunction()->name() == functionName.string();
                    });
                    if (it == availableFunctions.end()) {
                        PANIC(fmt::format("No variable or function named '{}'", functionName.string()));
                    }
                    stack.push_back(*it);
                    break;
                }
                case Op::LoadLocal: {
                    const auto& localIndex = constantList[constantIndex++] + localIndexOffsetStack.back();
                    const auto& localValue = localVariables[localIndex.value<usize>()];
                    stack.push_back(localValue);
                    break;
                }
                case Op::PopLocals: {
                    const auto& numLocals = constantList[constantIndex++];
                    for (auto i = 0zu; i < numLocals.value<usize>(); i++) {
                        localVariables.pop_back();
                    }
                    break;
                }
                case Op::PrintLn: {
                    const auto value = pop();
                    value.printLn();
                    break;
                }
                case Op::LogicOr: {
                    const auto [a, b] = popTwo();
                    stack.emplace_back(a || b);
                    break;
                }
                case Op::LogicAnd: {
                    const auto [a, b] = popTwo();
                    stack.emplace_back(a && b);
                    break;
                }
                case Op::BitwiseOr: {
                    const auto [a, b] = popTwo();
                    stack.emplace_back(a | b);
                    break;
                }
                case Op::BitwiseXor: {
                    const auto [a, b] = popTwo();
                    stack.emplace_back(a ^ b);
                    break;
                }
                case Op::BitwiseAnd: {
                    const auto [a, b] = popTwo();
                    stack.emplace_back(a & b);
                    break;
                }
                case Op::Equal: {
                    const auto [a, b] = popTwo();
                    stack.emplace_back(a == b);
                    break;
                }
                case Op::NotEqual: {
                    const auto [a, b] = popTwo();
                    stack.emplace_back(a != b);
                    break;
                }
                case Op::LessThan: {
                    const auto [a, b] = popTwo();
                    stack.emplace_back(a < b);
                    break;
                }
                case Op::LessEqual: {
                    const auto [a, b] = popTwo();
                    stack.emplace_back(a <= b);
                    break;
                }
                case Op::GreaterThan: {
                    const auto [a, b] = popTwo();
                    stack.emplace_back(a > b);
                    break;
                }
                case Op::GreaterEqual: {
                    const auto [a, b] = popTwo();
                    stack.emplace_back(a >= b);
                    break;
                }
                case Op::LeftShift: {
                    const auto [a, b] = popTwo();
                    stack.emplace_back(a << b);
                    break;
                }
                case Op::RightShift: {
                    const auto [a, b] = popTwo();
                    stack.emplace_back(a >> b);
                    break;
                }
                case Op::Addition: {
                    const auto [a, b] = popTwo();
                    stack.emplace_back(a + b);
                    break;
                }
                case Op::Subtraction: {
                    const auto [a, b] = popTwo();
                    stack.emplace_back(a - b);
                    break;
                }
                case Op::Multiply: {
                    const auto [a, b] = popTwo();
                    stack.emplace_back(a * b);
                    break;
                }
                case Op::Divide: {
                    const auto [a, b] = popTwo();
                    stack.emplace_back(a / b);
                    break;
                }
                case Op::Modulus: {
                    const auto [a, b] = popTwo();
                    stack.emplace_back(a % b);
                    break;
                }
                case Op::LogicNot: {
                    const auto value = pop();
                    stack.emplace_back(!value);
                    break;
                }
                case Op::BitwiseNot: {
                    const auto value = pop();
                    stack.emplace_back(~value);
                    break;
                }
                case Op::Negate: {
                    const auto value = pop();
                    stack.emplace_back(-value);
                    break;
                }
                case Op::Plus: {
                    const auto value = pop();
                    stack.emplace_back(+value);
                    break;
                }
                case Op::Call: {
                    localIndexOffsetStack.push_back(localVariables.size());

                    const auto numArgs = constantList[constantIndex++].value<usize>();;
                    localVariables.resize(localVariables.size() + numArgs);

                    for (auto i = 0zu; i < numArgs; i++) {
                        localVariables[localVariables.size() - 1zu - i] = pop();
                    }

                    const auto value = pop();

                    if (value.callable()) {
                        const auto function = value.object()->asFunction();
                        if (function->arity() != numArgs) {
                            PANIC(fmt::format("Function '{}' takes {} args but was given {}", function->name(), function->arity(), numArgs));
                        }

                        opListStack.push_back(function->opList());
                        constantListStack.push_back(function->constantList());
                        opIndexStack.push_back(0zu);
                        constantIndexStack.push_back(0zu);
                    } else {
                        PANIC(fmt::format("{} is not callable", value));
                    }

                    break;
                }
                case Op::Exit: {
                    return RunResult::Success;
                }
                case Op::Jump: {
                    const auto& jumpIndex = constantList[constantIndex++];
                    opIndex = jumpIndex.value<usize>();
                    break;
                }
                case Op::JumpIfFalse: {
                    POISE_ASSERT(!stack.empty(), "Stack should not be empty, there has been an error in codegen");

                    const auto& value = stack.back();
                    const auto& jumpConstantIndex = constantList[constantIndex++];
                    const auto& jumpOpIndex = constantList[constantIndex++];

                    if (!value.asBool()) {
                        constantIndex = jumpConstantIndex.value<usize>();
                        opIndex = jumpOpIndex.value<usize>();
                    }

                    break;
                }
                case Op::JumpIfTrue: {
                    POISE_ASSERT(!stack.empty(), "Stack should not be empty, there has been an error in codegen");

                    const auto& value = stack.back();
                    const auto& jumpConstantIndex = constantList[constantIndex++];
                    const auto& jumpOpIndex = constantList[constantIndex++];

                    if (value.asBool()) {
                        constantIndex = jumpConstantIndex.value<usize>();
                        opIndex = jumpOpIndex.value<usize>();
                    }

                    break;
                }
                case Op::Return: {
                    opListStack.pop_back();
                    constantListStack.pop_back();
                    opIndexStack.pop_back();
                    constantIndexStack.pop_back();
                    localIndexOffsetStack.pop_back();
                    break;
                }
            }
        }
    }
}
