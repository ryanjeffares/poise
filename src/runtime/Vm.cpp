#include "Vm.hpp"
#include "../objects/Objects.hpp"
#include "../scanner/Scanner.hpp"

#include <fmt/color.h>
#include <fmt/core.h>

#include <ranges>
#include <stack>

namespace poise::runtime {
using objects::PoiseException;

Vm::Vm(std::string mainFilePath)
    : m_mainFilePath{std::move(mainFilePath)}
    , m_typeLookup{
        {types::Type::Bool, Value::createObject<objects::PoiseType>(types::Type::Bool, "Bool")},
        {types::Type::Float, Value::createObject<objects::PoiseType>(types::Type::Float, "Float")},
        {types::Type::Int, Value::createObject<objects::PoiseType>(types::Type::Int, "Int")},
        {types::Type::None, Value::createObject<objects::PoiseType>(types::Type::None, "None")},
        {types::Type::String, Value::createObject<objects::PoiseType>(types::Type::String, "String")},
        {types::Type::Exception, Value::createObject<objects::PoiseType>(types::Type::Exception, "Exception")},
        {types::Type::Function, Value::createObject<objects::PoiseType>(types::Type::Function, "Function")},
        {types::Type::Iterator, Value::createObject<objects::PoiseType>(types::Type::Iterator, "Iterator")},
        {types::Type::List, Value::createObject<objects::PoiseType>(types::Type::List, "List")},
        {types::Type::Range, Value::createObject<objects::PoiseType>(types::Type::Range, "Range")},
        {types::Type::Type, Value::createObject<objects::PoiseType>(types::Type::Type, "Type")},
    }

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

auto Vm::namespaceManager() const noexcept -> const NamespaceManager*
{
    return &m_namespaceManager;
}

auto Vm::namespaceManager() noexcept -> NamespaceManager*
{
    return &m_namespaceManager;
}

auto Vm::typeValue(types::Type type) const noexcept -> const Value&
{
    return m_typeLookup.at(type);
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

auto Vm::run() noexcept -> RunResult
{
    std::vector<Value> stack;
    std::vector<Value> localVariables;

    struct CallStackEntry
    {
        usize localIndexOffset;
        usize opIndex;
        usize constantIndex;

        usize heldIteratorsSize;

        usize callSiteLine;

        objects::PoiseFunction* callerFunction;
        objects::PoiseFunction* calleeFunction;
    };

    std::vector<CallStackEntry> callStack{{
        .localIndexOffset = 0_uz,
        .opIndex = 0_uz,
        .constantIndex = 0_uz,
        .heldIteratorsSize = 0_uz,
        .callSiteLine = 0_uz,
        .callerFunction = nullptr,
        .calleeFunction = nullptr,
    }};

    struct TryBlockState    // TODO: better name
    {
        usize stackSize;
        usize callStackSize;
        usize constantIndexToJumpTo;
        usize opIndexToJumpTo;
        usize heldIteratorsSize;
    };

    std::stack<TryBlockState> tryBlockStateStack;
    std::stack<Value> heldIterators;

    auto pop = [&stack] () -> Value {
        POISE_ASSERT(!stack.empty(), "Stack is empty, there has been an error in codegen");
        auto value = std::move(stack.back());
        stack.pop_back();
        return value;
    };

    auto popTwo = [&stack] () -> std::tuple<Value, Value> {
        POISE_ASSERT(stack.size() >= 2_uz, "Stack is not big enough, there has been an error in codegen");
        auto value1 = std::move(stack.back());
        stack.pop_back();
        auto value2 = std::move(stack.back());
        stack.pop_back();
        return {std::move(value2), std::move(value1)};
    };

    auto popThree = [&stack] () -> std::tuple<Value, Value, Value> {
        POISE_ASSERT(stack.size() >= 3_uz, "Stack is not big enough, there has been an error in codegen");
        auto value1 = std::move(stack.back());
        stack.pop_back();
        auto value2 = std::move(stack.back());
        stack.pop_back();
        auto value3 = std::move(stack.back());
        stack.pop_back();
        return {std::move(value3), std::move(value2), std::move(value1)};
    };

    auto popCallArgs = [&pop] (usize numArgs) -> std::vector<Value> {
        std::vector<Value> args;
        args.resize(numArgs);

        for (auto i = 0_uz; i < numArgs; i++) {
            args[args.size() - 1_uz - i] = pop();
        }

        return args;
    };

#ifdef POISE_DEBUG
    auto printMemory = [&stack, &localVariables] {
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
                    auto numArgs = constantList[constantIndex++].value<usize>();
                    const auto hasUnpack = constantList[constantIndex++].value<bool>();

                    if (hasUnpack) {
                        numArgs += pop().value<usize>() - 1_uz; // -1 for the pack, replace it with the size of the pack
                    }

                    auto args = popCallArgs(numArgs);

                    if (type == types::Type::Range) {
                        const auto inclusiveRange = constantList[constantIndex++].value<bool>();
                        args.emplace_back(inclusiveRange);
                    }

                    stack.emplace_back(typeValue(type).object()->asType()->construct(args));
                    break;
                }
                case Op::DeclareLocal: {
                    auto value = pop();
                    localVariables.emplace_back(std::move(value));
                    break;
                }
                case Op::DeclareMultipleLocals: {
                    const auto numDeclarations = constantList[constantIndex++].value<usize>();
                    const auto numUnpacked = pop().value<usize>();

                    if (numDeclarations != numUnpacked) {
                        throw PoiseException(
                            PoiseException::ExceptionType::IncorrectArgCount,
                            fmt::format("Expected {} values to assign but unpacked {}", numDeclarations, numUnpacked)
                        );
                    }

                    auto unpacked = popCallArgs(numUnpacked);
                    for (auto i = 0_uz; i < unpacked.size(); i++) {
                        localVariables.emplace_back(std::move(unpacked[i]));
                    }
                    break;
                }
                case Op::EnterTry: {
                    const auto constantIndexToJumpTo = constantList[constantIndex++].value<usize>();
                    const auto opIndexToJumpTo = constantList[constantIndex++].value<usize>();

                    tryBlockStateStack.push({
                        .stackSize = stack.size(),
                        .callStackSize = callStack.size(),
                        .constantIndexToJumpTo = constantIndexToJumpTo,
                        .opIndexToJumpTo = opIndexToJumpTo,
                        .heldIteratorsSize = heldIterators.size(),
                    });
                    break;
                }
                case Op::ExitTry: {
                    tryBlockStateStack.pop();
                    break;
                }
                case Op::LoadCapture: {
                    // captures need to be inserted before call args
                    const auto index = constantList[constantIndex++].value<usize>();
                    const auto& capture = currentFunction->getCapture(index);
                    const auto insertionIdx = localVariables.size() - currentFunction->arity();
                    localVariables.insert(localVariables.begin() + static_cast<isize>(insertionIdx), capture);
                    break;
                }
                case Op::LoadConstant: {
                    stack.push_back(constantList[constantIndex++]);
                    break;
                }
                case Op::LoadFunction: {
                    const auto namespaceHash = constantList[constantIndex++].value<NamespaceManager::NamespaceHash>();
                    const auto functionNameHash = constantList[constantIndex++].value<usize>();
                    const auto& functionName = constantList[constantIndex++].string();

                    if (auto function = m_namespaceManager.namespaceFunction(namespaceHash, functionNameHash)) {
                        stack.push_back(std::move(*function));
                    } else {
                        throw PoiseException(PoiseException::ExceptionType::FunctionNotFound, fmt::format("Function '{}' not found in namespace '{}'", functionName, m_namespaceManager.namespaceDisplayName(namespaceHash)));
                    }

                    break;
                }
                case Op::LoadLocal: {
                    const auto& localIndex = constantList[constantIndex++];
                    const auto& localValue = localVariables[localIndex.value<usize>() + localIndexOffset];
                    stack.push_back(localValue);
                    break;
                }
                case Op::LoadMember: {
                    // TODO: class member variables
                    auto value = pop();
                    const auto type = typeValue(value.type()).object()->asType();
                    const auto& memberName = constantList[constantIndex++].string();
                    const auto& memberNameHash = constantList[constantIndex++].value<usize>();
                    const auto pushParentBack = constantList[constantIndex++].toBool();

                    if (auto function = type->findExtensionFunction(memberNameHash)) {
                        const auto p = function->object()->asFunction();
                        if (currentFunction->namespaceHash() != p->namespaceHash()) {
                            if (!m_namespaceManager.namespaceHasImportedNamespace(currentFunction->namespaceHash(), p->namespaceHash())) {
                                throw PoiseException(PoiseException::ExceptionType::FunctionNotFound, fmt::format("Extension function '{}' not found for type '{}' - are you missing an import?", p->name(), type->typeName()));
                            }
                        }
                        stack.push_back(std::move(*function));
                        if (pushParentBack) {
                            stack.push_back(std::move(value));
                        }
                    } else {
                        throw PoiseException(PoiseException::ExceptionType::FunctionNotFound, fmt::format("Function '{}' not defined for type '{}'", memberName, type->typeName()));
                    }
                    break;
                }
                case Op::LoadType: {
                    const auto type = static_cast<types::Type>(constantList[constantIndex++].value<u8>());
                    stack.push_back(typeValue(type));
                    break;
                }
                case Op::PopLocals: {
                    const auto numLocalsToRemain = constantList[constantIndex++].value<usize>();
                    localVariables.resize(numLocalsToRemain + localIndexOffset);
                    break;
                }
                case Op::Pop: {
                    pop();
                    break;
                }
                case Op::Throw: {
                    auto value = pop();
                    if (value.type() != types::Type::Exception) {
                        throw PoiseException(PoiseException::ExceptionType::InvalidType, fmt::format("Only Exceptions can be thrown"));
                    }

                    const auto exception = value.object()->asException();
                    throw PoiseException(exception->exceptionType(), std::string{exception->message()});
                }
                case Op::Unpack: {
                    auto value = pop();
                    if (value.object() == nullptr || value.object()->asIterable() == nullptr) {
                        throw PoiseException(PoiseException::ExceptionType::InvalidType, fmt::format("{} cannot be unpacked", value.type()));
                    }
                    value.object()->asIterable()->unpack(stack);
                    break;
                }
                case Op::TypeOf: {
                    stack.emplace_back(typeValue(pop().type()));
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
                case Op::MakeList: {
                    auto numArgs = constantList[constantIndex++].value<usize>();
                    const auto hasUnpack = constantList[constantIndex++].value<bool>();

                    if (hasUnpack) {
                        numArgs += pop().value<usize>() - 1_uz; // -1 for the pack, replace it with the size of the pack
                    }

                    stack.emplace_back(Value::createObject<objects::iterables::PoiseList>(popCallArgs(numArgs)));
                    break;
                }
                case Op::MakeLambda: {
                    const auto lambda = constantList[constantIndex++].object()->asFunction();
                    stack.emplace_back(lambda->shallowClone());
                    break;
                }
                case Op::AssignIndex: {
                    auto [collection, index, value] = popThree();
                    switch (collection.type()) {
                        case types::Type::List: {
                            if (index.type() != types::Type::Int) {
                                throw PoiseException(
                                    PoiseException::ExceptionType::InvalidType,
                                    fmt::format("Expected Int to index List but got {}", index.type())
                                );
                            }

                            collection.object()->asList()->at(index.value<isize>()) = std::move(value);
                            break;
                        }
                        default: {
                            throw PoiseException(
                                PoiseException::ExceptionType::InvalidType,
                                fmt::format("Cannot assign to {} at index", collection.type())
                            );
                        }
                    }
                    break;
                }
                case Op::LoadIndex: {
                    auto [collection, index] = popTwo();
                    switch (collection.type()) {
                        case types::Type::List: {
                            if (index.type() != types::Type::Int) {
                                throw PoiseException(
                                    PoiseException::ExceptionType::InvalidType,
                                    fmt::format("Expected Int to index List but got {}", index.type())
                                );
                            }

                            stack.push_back(collection.object()->asList()->at(index.value<isize>()));
                            break;
                        }
                        case types::Type::String: {
                            if (index.type() != types::Type::Int) {
                                throw PoiseException(
                                    PoiseException::ExceptionType::InvalidType,
                                    fmt::format("Expected Int to index String but got {}", index.type())
                                );
                            }

                            const auto& s = collection.string();
                            const auto i = index.value<isize>(); 
                            if (i < 0_i64 || i >= std::ssize(s)) {
                                throw PoiseException(
                                    PoiseException::ExceptionType::IndexOutOfBounds,
                                    fmt::format("The index is {} but the size is {}", i, s.size())
                                );
                            }

                            std::string res;
                            res.push_back(s[static_cast<usize>(i)]);
                            stack.emplace_back(std::move(res));
                            break;
                        }
                        default: {
                            throw PoiseException(
                                PoiseException::ExceptionType::InvalidType,
                                fmt::format("Cannot index {}", collection.type())
                            );
                        }
                    }
                    break;
                }
                case Op::Call: {
                    auto numArgs = constantList[constantIndex++].value<usize>();
                    const auto hasUnpack = constantList[constantIndex++].value<bool>();

                    if (hasUnpack) {
                        numArgs += pop().value<usize>() - 1_uz; // -1 for the pack, replace it with the size of the pack
                    }

                    auto args = popCallArgs(numArgs);   // not const so we can move into local vars if needed

                    const auto isDotCall = constantList[constantIndex++].value<bool>();
                    if (isDotCall) {
                        args.insert(args.begin(), pop());
                    }

                    const auto function = pop();

                    if (auto object = function.object()) {
                        if (auto calleeFunction = object->asFunction()) {
                            const auto hasVariadicParams = calleeFunction->hasVariadicParams();

                            // if the function has a pack, numParams can be >= arity
                            numArgs = isDotCall ? numArgs + 1_u8 : numArgs;
                            if (hasVariadicParams) {
                                if (numArgs < calleeFunction->arity()) {
                                    throw PoiseException(
                                        PoiseException::ExceptionType::IncorrectArgCount,
                                        fmt::format("Function '{}' takes >={} args but was given {}", calleeFunction->name(), calleeFunction->arity(), numArgs)
                                    );
                                }
                            } else {
                                if (numArgs != calleeFunction->arity()) {
                                    throw PoiseException(
                                        PoiseException::ExceptionType::IncorrectArgCount,
                                        fmt::format("Function '{}' takes {} args but was given {}", calleeFunction->name(), calleeFunction->arity(), numArgs)
                                    );
                                }
                            }

                            callStack.push_back({
                                .localIndexOffset = localVariables.size(),
                                .opIndex = 0_uz,
                                .constantIndex = 0_uz,
                                .heldIteratorsSize = heldIterators.size(),
                                .callSiteLine = line,
                                .callerFunction = currentFunction,
                                .calleeFunction = calleeFunction,
                            });

                            if (hasVariadicParams) {
                                std::vector<Value> variadicParams;
                                for (auto i = calleeFunction->arity() - 1_uz; i < numArgs; i++) {
                                    variadicParams.emplace_back(std::move(args[i]));
                                }
                                args.resize(args.size() - variadicParams.size());
                                args.emplace_back(Value::createObject<objects::iterables::PoiseList>(std::move(variadicParams)));
                            }

                            localVariables.insert(localVariables.end(), std::make_move_iterator(args.begin()), std::make_move_iterator(args.end()));
                        } else if (auto type = object->asType()) {
                            stack.emplace_back(type->construct(args));
                        } else {
                            throw PoiseException(PoiseException::ExceptionType::InvalidType, fmt::format("{} is not callable", function));
                        }
                    } else {
                        throw PoiseException(PoiseException::ExceptionType::InvalidType, fmt::format("{} is not callable", function.type()));
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
                    POISE_ASSERT(heldIterators.empty(), "Held iterators not empty, there has been an error in codegen");
                    POISE_ASSERT(tryBlockStateStack.empty(), "Try block state stack not empty, there has been an error in codegen");
                    POISE_ASSERT(callStack.size() == 1_uz, "Call stack not empty, there has been an error in codegen");
                    return RunResult::Success;
                }
                case Op::InitIterator: {
                    auto value = pop();
                    if (value.object() == nullptr || !value.object()->iterable()) {
                        throw PoiseException(PoiseException::ExceptionType::InvalidType, fmt::format("{} is not iterable", value.type()));
                    }

                    heldIterators.push(Value::createObject<objects::iterables::PoiseIterator>(value));
                    auto iteratorPtr = heldIterators.top().object()->asIterator();

                    const auto firstIteratorLocalIndex = constantList[constantIndex++].value<usize>();
                    const auto secondIteratorLocalIndex = constantList[constantIndex++].value<usize>();

                    localVariables[firstIteratorLocalIndex + localIndexOffset] = iteratorPtr->isAtEnd() ? Value::none() : iteratorPtr->value();
                    if (secondIteratorLocalIndex > 0_uz) {
                        // even if there are no local variables before the loop, the second iterator would have an index of 1
                        // so, we can do this check instead of some extra bool flag
                        if (value.type() == types::Type::List) {
                            localVariables[secondIteratorLocalIndex + localIndexOffset] = 0_i64;
                        } else {
                            throw PoiseException(PoiseException::ExceptionType::InvalidType, fmt::format("{} cannot have two iterators", value.type()));
                        }
                    }

                    stack.emplace_back(iteratorPtr->isAtEnd());
                    break;
                }
                case Op::IncrementIterator: {
                    auto iterator = heldIterators.top().object()->asIterator();
                    iterator->increment();
                    stack.emplace_back(iterator->isAtEnd());
                    const auto firstIteratorLocalIndex = constantList[constantIndex++].value<usize>();
                    const auto secondIteratorLocalIndex = constantList[constantIndex++].value<usize>();
                    if (iterator->isAtEnd()) {
                        localVariables[firstIteratorLocalIndex + localIndexOffset] = Value::none();
                        heldIterators.pop();
                    } else {
                        localVariables[firstIteratorLocalIndex + localIndexOffset] = iterator->value();
                    }

                    if (secondIteratorLocalIndex > 0_uz) {
                        auto& local = localVariables[secondIteratorLocalIndex + localIndexOffset];
                        // type was checked in InitIterator
                        if (iterator->iterableValue().type() == types::Type::List) {
                            local = local.value<i64>() + 1_i64;
                        }
                    }
                    break;
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
                    while (heldIterators.size() != callStack.back().heldIteratorsSize) {
                        heldIterators.pop();
                    }
                    callStack.pop_back();
                    break;
                }
            }
        } catch (const PoiseException& exception) {
            const auto inTryBlock = !tryBlockStateStack.empty();

            if (inTryBlock) {
                const auto [stackSize, callStackSize, constantIndexToJumpTo, opIndexToJumpTo, heldIteratorsSize] = tryBlockStateStack.top();

                callStack.resize(callStackSize);
                callStack.back().constantIndex = constantIndexToJumpTo;
                callStack.back().opIndex = opIndexToJumpTo;

                while (heldIterators.size() != heldIteratorsSize) {
                    heldIterators.pop();
                }

                tryBlockStateStack.pop();

                stack.resize(stackSize);
                stack.push_back(Value::createObject<PoiseException>(exception.exceptionType(), std::string{exception.message()}));
            } else {
                fmt::print(stderr, fmt::emphasis::bold | fmt::fg(fmt::color::red), "Runtime Error: ");
                fmt::print(stderr, "{}\n", exception.toString());

                fmt::print(stderr, "  At {}:{} in function '{}'\n", currentFunction->filePath().string(), line, currentFunction->name());
                fmt::print(stderr, "    {}\n", scanner::Scanner::getCodeAtLine(currentFunction->filePath(), line));

                for (const auto& entry : callStack | std::views::reverse) {
                    if (const auto caller = entry.callerFunction) {
                        fmt::print(stderr, "  At {}:{} in function '{}'\n", caller->filePath().string(), entry.callSiteLine, caller->name());
                        fmt::print(stderr, "    {}\n", scanner::Scanner::getCodeAtLine(caller->filePath(), entry.callSiteLine));
                    }
                }

                fmt::print(stderr, "\nThis is an exception thrown by the runtime as a result of a problem in your poise code that has not been caught.\n");
                fmt::print(stderr, "Consider reviewing your code or catching this exception with a `try/catch` statement.\n");
                return RunResult::RuntimeError;
            }
        }/* catch (const std::exception& exception) {
            fmt::print(stderr, fmt::emphasis::bold | fmt::fg(fmt::color::red), "PANIC: ");
            fmt::print(stderr, "{}\n", exception.what());
            fmt::print(stderr, "This is an error that cannot be recovered from or caught, and is likely a bug in the interpreter.\n");
            return RunResult::RuntimeError;
        }*/
    }
#undef PRINT_MEMORY
}
}   // namespace poise::runtime
