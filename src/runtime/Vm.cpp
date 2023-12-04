#include "Vm.hpp"
#include "../objects/PoiseException.hpp"
#include "../objects/PoiseType.hpp"

#include <fmt/color.h>
#include <fmt/core.h>

namespace poise::runtime {
using objects::PoiseException;

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
#ifdef POISE_DEBUG
#define PRINT_STACK()                           \
        do {                                    \
            fmt::print("STACK:\n");             \
            for (const auto& value : stack) {   \
                fmt::print("\t{}\n", value);    \
            }                                   \
        }                                       \
        while (false)
#else
#define PRINT_STACK()
#endif

    std::vector<Value> stack;
    std::vector<Value> localVariables;
    std::vector<Value> availableFunctions;

    struct CallStackEntry
    {
        std::span<const OpLine> opList;
        std::span<const Value> constantList;

        usize localIndexOffset;
        usize opIndex;
        usize constantIndex;

        usize line;

        objects::PoiseFunction* function;
    };

    std::vector<CallStackEntry> callStack{{
        .opList = m_globalOps,
        .constantList = m_globalConstants,
        .localIndexOffset = 0_uz,
        .opIndex = 0_uz,
        .constantIndex = 0_uz,
        .line = 0_uz,
        .function = nullptr,
    }};

    struct VmState
    {
        usize stackSize{}, numLocals{};
    };

    // used to restore the state of the call stack after an exception is caught
    std::vector<usize> callStackSizeStack;

    auto pop = [&stack] -> Value {
        POISE_ASSERT(!stack.empty(), "Stack is empty, there has been an error in codegen");
        auto value = std::move(stack.back());
        stack.pop_back();
        return value;
    };

    auto popTwo = [&stack] -> std::tuple<Value, Value> {
        POISE_ASSERT(stack.size() >= 2_uz, "Stack is not big enough, there has been an error in codegen");
        auto value1 = std::move(stack.back());
        stack.pop_back();
        auto value2 = std::move(stack.back());
        stack.pop_back();
        return {std::move(value2), std::move(value1)};
    };

    auto popCallArgs = [&pop] [[nodiscard]](u8 numArgs) -> std::vector<Value> {
        std::vector<Value> args;
        args.resize(numArgs);

        for (auto i = 0_uz; i < numArgs; i++) {
            args[args.size() - 1_uz - i] = pop();
        }

        return args;
    };

    while (true) {
        auto& callStackTop = callStack.back();

        const auto opList = callStackTop.opList;
        const auto constantList = callStackTop.constantList;
        const auto localIndexOffset = callStackTop.localIndexOffset;

        auto& opIndex = callStackTop.opIndex;
        auto& constantIndex = callStackTop.constantIndex;

        auto currentFunction = callStackTop.function;

        const auto [op, line] = opList[opIndex++];

        try {
            switch (op) {
                case Op::AssignLocal: {
                    const auto index = constantList[constantIndex++].value<usize>();
                    localVariables[index + localIndexOffset] = pop();
                    break;
                }
                case Op::CaptureLocal: {
                    auto& lambda = stack.back();
                    const auto index = constantList[constantIndex++].value<usize>();
                    const auto& local = localVariables[index + localIndexOffset];
                    lambda.object()->asFunction()->addCapture(local);
                    break;
                }
                case Op::ConstructBuiltin: {
                    const auto type = static_cast<types::Type>(constantList[constantIndex++].value<u8>());
                    const auto numArgs = constantList[constantIndex++].value<u8>();
                    const auto args = popCallArgs(numArgs);
                    stack.emplace_back(types::s_typeLookup.at(type).object()->asType()->construct(args));
                    break;
                }
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
                case Op::EnterTry: {
                    callStackSizeStack.push_back(callStack.size());
                    break;
                }
                case Op::LoadCapture: {
                    const auto index = constantList[constantIndex++].value<usize>();
                    const auto& capture = currentFunction->getCapture(index);
                    localVariables.emplace_back(capture);
                    break;
                }
                case Op::LoadConstant: {
                    stack.push_back(constantList[constantIndex++]);
                    break;
                }
                case Op::LoadFunction: {
                    const auto& functionName = constantList[constantIndex++];
                    const auto it = std::find_if(availableFunctions.begin(), availableFunctions.end(), [&functionName](const Value& function) {
                        return function.object()->asFunction()->name() == functionName.string();
                    });
                    if (it == availableFunctions.end()) {
                        throw PoiseException(PoiseException::ExceptionType::FunctionNotFound, fmt::format("No variable or function named '{}'", functionName.string()));
                    }
                    stack.push_back(*it);
                    break;
                }
                case Op::LoadLocal: {
                    const auto& localIndex = constantList[constantIndex++];
                    const auto& localValue = localVariables[localIndex.value<usize>() + localIndexOffset];
                    stack.push_back(localValue);
                    break;
                }
                case Op::LoadType: {
                    const auto type = static_cast<types::Type>(constantList[constantIndex++].value<u8>());
                    stack.push_back(types::s_typeLookup.at(type));
                    break;
                }
                case Op::PopLocals: {
                    const auto& numLocals = constantList[constantIndex++];
                    for (auto i = 0_uz; i < numLocals.value<usize>(); i++) {
                        localVariables.pop_back();
                    }
                    break;
                }
                case Op::Pop: {
                    pop();
                    break;
                }
                case Op::TypeOf: {
                    stack.emplace_back(pop().typeValue());
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
                    const auto numArgs = constantList[constantIndex++].value<u8>();
                    auto args = popCallArgs(numArgs);   // not const so we can move into local vars if needed
                    const auto value = pop();

                    if (auto object = value.object()) {
                        if (auto function = object->asFunction()) {
                            if (function->arity() != numArgs) {
                                throw PoiseException(PoiseException::ExceptionType::IncorrectArgCount, fmt::format("Function '{}' takes {} args but was given {}", function->name(), function->arity(), numArgs));
                            }

                            callStack.push_back({
                                .opList = function->opList(),
                                .constantList = function->constantList(),
                                .localIndexOffset = localVariables.size(),
                                .opIndex = 0_uz,
                                .constantIndex = 0_uz,
                                .line = line,
                                .function = function,
                            });

                            localVariables.insert(localVariables.end(), std::make_move_iterator(args.begin()), std::make_move_iterator(args.end()));
                        } else if (auto type = object->asType()) {
                            stack.emplace_back(type->construct(args));
                        } else {
                            throw PoiseException(PoiseException::ExceptionType::InvalidType, fmt::format("{} is not callable", value));
                        }
                    } else {
                        throw PoiseException(PoiseException::ExceptionType::InvalidType, fmt::format("{} is not callable", value));
                    }

                    break;
                }
                case Op::Exit: {
                    POISE_ASSERT(stack.empty(), "Stack not empty after runtime, there has been an error in codegen");
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

                    if (!value.toBool()) {
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

                    if (value.toBool()) {
                        constantIndex = jumpConstantIndex.value<usize>();
                        opIndex = jumpOpIndex.value<usize>();
                    }

                    break;
                }
                case Op::Return: {
                    PRINT_STACK();
                    callStack.pop_back();
                    break;
                }
            }
        } catch (const objects::PoiseException& exception) {
            auto inTryBlock = !callStackSizeStack.empty();

            if (!inTryBlock) {
                fmt::print(stderr, fmt::emphasis::bold | fmt::fg(fmt::color::red), "Runtime Error: ");
                fmt::print(stderr, "{}\n", exception.toString());
                fmt::print(stderr, "This is an exception thrown by the runtime as a result of a problem in your poise code that has not been caught.\n");
                fmt::print(stderr, "Consider reviewing your code or catching this exception with a `try/catch` statement.\n");
                return RunResult::RuntimeError;
            }

            // handle
            const auto callStackSizeNeeded = callStackSizeStack.back();
            callStackSizeStack.pop_back();
            inTryBlock = !callStackSizeStack.empty();

            callStack.resize(callStackSizeNeeded);
        } catch (const std::exception& exception) {
            fmt::print(stderr, fmt::emphasis::bold | fmt::fg(fmt::color::red), "PANIC: ");
            fmt::print(stderr, "{}\n", exception.what());
            fmt::print(stderr, "This is an error that cannot be recovered from or caught, and is likely a bug in the interpreter.\n");
            return RunResult::RuntimeError;
        }
    }
#undef PRINT_STACK
}
}   // namespace poise::runtime
