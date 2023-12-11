#include "Vm.hpp"
#include "../objects/PoiseException.hpp"
#include "../objects/PoiseType.hpp"

#include <fmt/color.h>
#include <fmt/core.h>

#include <ranges>
#include <stack>

namespace poise::runtime {
using objects::PoiseException;

Vm::Vm(std::string mainFilePath) : m_mainFilePath{std::move(mainFilePath)}
{
    registerNatives();
}

auto Vm::setCurrentFunction(objects::PoiseFunction* function) noexcept -> void
{
    m_currentFunction = function;
}

auto Vm::currentFunction() const noexcept -> objects::PoiseFunction*
{
    return m_currentFunction;
}

auto Vm::nativeFunctionHash(std::string_view functionName) const noexcept -> std::optional<NativeNameHash>
{
    const auto hash = m_nativeNameHasher(functionName);
    return m_nativeFunctionLookup.contains(hash) ? std::optional{hash} : std::nullopt;
}

auto Vm::nativeFunctionArity(NativeNameHash hash) const noexcept -> u8
{
    return m_nativeFunctionLookup.at(hash).arity();
}

auto Vm::addNamespace(const std::filesystem::path& namespacePath, std::string namespaceName, std::optional<NamespaceHash> parent) noexcept -> bool
{
    const auto hash = namespaceHash(namespacePath);

    // sort out namespaces' imported namespaces first
    // hacky....
    if (!parent) {
        // main file compiler's constructor, needs to just be present in the map
        m_namespacesImportedToNamespaceLookup.try_emplace(hash, std::vector<NamespaceHash>{});
    } else {
        // actual import declaration
        // parent file knows its import, imported file becomes present in the map
        m_namespacesImportedToNamespaceLookup[*parent].push_back(hash);
        m_namespacesImportedToNamespaceLookup.try_emplace(hash, std::vector<NamespaceHash>());
    }

    // then return false if we've already compiled this file
    if (m_namespaceFunctionLookup.contains(hash)) {
        return false;
    }

    m_namespaceFunctionLookup.try_emplace(hash, std::vector<Value>{});
    m_namespaceNameMap[hash] = std::move(namespaceName);

    return true;
}

auto Vm::hasNamespace(NamespaceHash namespaceHash) const noexcept -> bool
{
    return m_namespaceFunctionLookup.contains(namespaceHash);
}

auto Vm::namespaceHash(const std::filesystem::path& namespacePath) const noexcept -> NamespaceHash
{
    return m_namespaceHasher(namespacePath);
}

auto Vm::addFunctionToNamespace(NamespaceHash namespaceHash, Value function) noexcept -> void
{
    POISE_ASSERT(m_namespaceFunctionLookup.contains(namespaceHash), "Namespace not found");
    m_namespaceFunctionLookup[namespaceHash].emplace_back(std::move(function));
}

auto Vm::namespaceFunction(NamespaceHash namespaceHash, std::string_view functionName) const noexcept -> objects::PoiseFunction*
{
    POISE_ASSERT(m_namespaceFunctionLookup.contains(namespaceHash), "Namespace not found");

    const auto& functionVec = m_namespaceFunctionLookup.at(namespaceHash);
    const auto it = std::find_if(functionVec.cbegin(), functionVec.cend(), [functionName] (const Value& value) {
        return value.object()->asFunction()->name() == functionName;
    });

    return it == functionVec.end() ? nullptr : it->object()->asFunction();
}

auto Vm::namespaceHasImportedNamespace(NamespaceHash parent, NamespaceHash imported) const noexcept -> bool
{
    POISE_ASSERT(m_namespacesImportedToNamespaceLookup.contains(parent), "Parent namespace not found");
    const auto& namespaceVec = m_namespacesImportedToNamespaceLookup.at(parent);
    return std::find(namespaceVec.cbegin(), namespaceVec.cend(), imported) != namespaceVec.cend();
}

auto Vm::emitOp(Op op, usize line) noexcept -> void
{
    if (m_currentFunction == nullptr) {
        m_globalOps.push_back({op, line});
    } else {
        m_currentFunction->emitOp(op, line);
    }
}

auto Vm::emitConstant(Value value) noexcept -> void
{
    if (m_currentFunction == nullptr) {
        m_globalConstants.emplace_back(std::move(value));
    } else {
        m_currentFunction->emitConstant(std::move(value));
    }
}

auto Vm::run(const scanner::Scanner* const scanner) noexcept -> RunResult
{
    std::vector<Value> stack;
    std::vector<Value> localVariables;

    struct CallStackEntry
    {
        usize localIndexOffset;
        usize opIndex;
        usize constantIndex;

        usize callSiteLine;

        objects::PoiseFunction* callerFunction;
        objects::PoiseFunction* calleeFunction;
    };

    std::vector<CallStackEntry> callStack{{
        .localIndexOffset = 0_uz,
        .opIndex = 0_uz,
        .constantIndex = 0_uz,
        .callSiteLine = 0_uz,
        .callerFunction = nullptr,
        .calleeFunction = nullptr,
    }};

    struct TryBlockState    // TODO: better name
    {
        usize callStackSize;
        usize constantIndexToJumpTo;
        usize opIndexToJumpTo;
    };

    std::stack<TryBlockState> tryBlockStateStack;

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

#ifdef POISE_DEBUG
    auto printMemory = [&stack, &localVariables] -> void {
        // I'm assuming this gets yeeted in release...
        fmt::print("STACK:\n");
        for (const auto& value : stack) {
            fmt::print("\t{}\n", value);
        }
        fmt::print("LOCALS:\n");
        for (const auto& local : localVariables) {
            fmt::print("\t{}\n", local);
        }
    };
#else
    auto printMemory = []{};
#endif

    while (true) {
        auto& callStackTop = callStack.back();

        const auto localIndexOffset = callStackTop.localIndexOffset;
        auto& opIndex = callStackTop.opIndex;
        auto& constantIndex = callStackTop.constantIndex;

        const auto currentFunction = callStackTop.calleeFunction;

        const auto opList = currentFunction ? currentFunction->opList() : m_globalOps;
        const auto constantList = currentFunction ? currentFunction->constantList() : m_globalConstants;

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
                case Op::DeclareLocal: {
                    auto value = pop();
                    localVariables.emplace_back(std::move(value));
                    break;
                }
                case Op::EnterTry: {
                    const auto constantIndexToJumpTo = constantList[constantIndex++].value<usize>();
                    const auto opIndexToJumpTo = constantList[constantIndex++].value<usize>();

                    tryBlockStateStack.push({
                        .callStackSize = callStack.size(),
                        .constantIndexToJumpTo = constantIndexToJumpTo,
                        .opIndexToJumpTo = opIndexToJumpTo,
                    });
                    break;
                }
                case Op::ExitTry: {
                    tryBlockStateStack.pop();
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
                    const auto namespaceHash = constantList[constantIndex++].value<NamespaceHash>();
                    const auto functionNameHash = constantList[constantIndex++].value<usize>();
                    const auto& functionName = constantList[constantIndex++].string();

                    const auto& functionVec = m_namespaceFunctionLookup[namespaceHash];
                    const auto it = std::find_if(functionVec.cbegin(), functionVec.cend(), [functionNameHash] (const Value& value) {
                        return value.object()->asFunction()->nameHash() == functionNameHash;
                    });

                    if (it == functionVec.cend()) {
                        throw PoiseException(PoiseException::ExceptionType::FunctionNotFound, fmt::format("Function '{}' not found in namespace '{}'", functionName, m_namespaceNameMap[namespaceHash]));
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
                    const auto numLocalsToPop = constantList[constantIndex++].value<usize>();
                    for (auto i = 0_uz; i < numLocalsToPop; i++) {
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
                case Op::Print: {
                    const auto value = pop();
                    const auto err = constantList[constantIndex++].value<bool>();
                    const auto newLine = constantList[constantIndex++].value<bool>();
                    value.print(err, newLine);
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
                        if (auto calleeFunction = object->asFunction()) {
                            if (calleeFunction->arity() != numArgs) {
                                throw PoiseException(PoiseException::ExceptionType::IncorrectArgCount, fmt::format("Function '{}' takes {} args but was given {}", calleeFunction->name(), calleeFunction->arity(), numArgs));
                            }

                            callStack.push_back({
                                .localIndexOffset = localVariables.size(),
                                .opIndex = 0_uz,
                                .constantIndex = 0_uz,
                                .callSiteLine = line,
                                .callerFunction = currentFunction,
                                .calleeFunction = calleeFunction,
                            });

                            localVariables.insert(localVariables.end(), std::make_move_iterator(args.begin()), std::make_move_iterator(args.end()));
                        } else if (auto type = object->asType()) {
                            stack.emplace_back(type->construct(args));
                        } else {
                            throw PoiseException(PoiseException::ExceptionType::InvalidType, fmt::format("{} is not callable", value));
                        }
                    } else {
                        throw PoiseException(PoiseException::ExceptionType::InvalidType, fmt::format("{} is not callable", value.type()));
                    }

                    break;
                }
                case Op::CallNative: {
                    const auto hash = constantList[constantIndex++].value<NativeNameHash>();
                    const auto function = m_nativeFunctionLookup.at(hash);
                    const auto arity = function.arity();
                    auto args = popCallArgs(arity); // number of call args is checked at compile time
                    stack.emplace_back(function(args));
                    break;
                }
                case Op::Exit: {
                    POISE_ASSERT(stack.empty(), "Stack not empty after runtime, there has been an error in codegen");
                    POISE_ASSERT(localVariables.empty(), "Locals have not been popped, there has been an error in codegen");
                    return RunResult::Success;
                }
                case Op::Jump: {
                    const auto& jumpConstantIndex = constantList[constantIndex++];
                    const auto& jumpOpIndex = constantList[constantIndex++];
                    callStackTop.constantIndex = jumpConstantIndex.value<usize>();
                    callStackTop.opIndex = jumpOpIndex.value<usize>();
                    break;
                }
                case Op::JumpIfFalse: {
                    POISE_ASSERT(!stack.empty(), "Stack should not be empty, there has been an error in codegen");

                    const auto& value = stack.back();
                    const auto& jumpConstantIndex = constantList[constantIndex++];
                    const auto& jumpOpIndex = constantList[constantIndex++];
                    const auto& popValue = constantList[constantIndex++];

                    if (!value.toBool()) {
                        callStackTop.constantIndex = jumpConstantIndex.value<usize>();
                        callStackTop.opIndex = jumpOpIndex.value<usize>();
                    }

                    if (popValue.toBool()) {
                        pop();
                    }

                    break;
                }
                case Op::JumpIfTrue: {
                    POISE_ASSERT(!stack.empty(), "Stack should not be empty, there has been an error in codegen");

                    const auto& value = stack.back();
                    const auto& jumpConstantIndex = constantList[constantIndex++];
                    const auto& jumpOpIndex = constantList[constantIndex++];
                    const auto& popIfJump = constantList[constantIndex++];

                    if (value.toBool()) {
                        callStackTop.constantIndex = jumpConstantIndex.value<usize>();
                        callStackTop.opIndex = jumpOpIndex.value<usize>();
                    }
                    
                    if (popIfJump.toBool()) {
                        pop();
                    }

                    break;
                }
                case Op::Return: {
                    printMemory();
                    callStack.pop_back();
                    break;
                }
            }
        } catch (const PoiseException& exception) {
            const auto inTryBlock = !tryBlockStateStack.empty();

            if (inTryBlock) {
                const auto [callStackSize, constantIndexToJumpTo, opIndexToJumpTo] = tryBlockStateStack.top();

                callStack.resize(callStackSize);
                callStack.back().constantIndex = constantIndexToJumpTo;
                callStack.back().opIndex = opIndexToJumpTo;

                tryBlockStateStack.pop();

                stack.push_back(Value::createObject<PoiseException>(exception.exceptionType(), std::string{exception.message()}));
            } else {
                fmt::print(stderr, fmt::emphasis::bold | fmt::fg(fmt::color::red), "Runtime Error: ");
                fmt::print(stderr, "{}\n", exception.toString());

                fmt::print(stderr, "  At {}:{} in function '{}'\n", currentFunction->filePath().string(), line, currentFunction->name());
                fmt::print(stderr, "    {}\n", scanner->getCodeAtLine(line));

                for (const auto& entry : callStack | std::views::reverse) {
                    if (const auto caller = entry.callerFunction) {
                        fmt::print(stderr, "  At {}:{} in function '{}'\n", caller->filePath().string(), entry.callSiteLine, caller->name());
                        fmt::print(stderr, "    {}\n", scanner->getCodeAtLine(entry.callSiteLine));
                    }
                }

                fmt::print(stderr, "\nThis is an exception thrown by the runtime as a result of a problem in your poise code that has not been caught.\n");
                fmt::print(stderr, "Consider reviewing your code or catching this exception with a `try/catch` statement.\n");
                return RunResult::RuntimeError;
            }
        } catch (const std::exception& exception) {
            fmt::print(stderr, fmt::emphasis::bold | fmt::fg(fmt::color::red), "PANIC: ");
            fmt::print(stderr, "{}\n", exception.what());
            fmt::print(stderr, "This is an error that cannot be recovered from or caught, and is likely a bug in the interpreter.\n");
            return RunResult::RuntimeError;
        }
    }
#undef PRINT_MEMORY
}
}   // namespace poise::runtime
