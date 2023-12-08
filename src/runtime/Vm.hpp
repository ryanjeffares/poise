#ifndef POISE_VM_HPP
#define POISE_VM_HPP

#include "../Poise.hpp"

#include "../objects/PoiseFunction.hpp"
#include "NativeFunction.hpp"
#include "Op.hpp"
#include "../scanner/Scanner.hpp"
#include "Value.hpp"

#include <unordered_map>
#include <utility>
#include <vector>

namespace poise::runtime {
class Vm
{
public:
    enum class RunResult
    {
        Success,
        InitError,
        RuntimeError,
    };

    using NativeNameHash = usize;
    using NativeFunctionMap = std::unordered_map<NativeNameHash, NativeFunction>;

    Vm(std::string mainFilePath);

    auto setCurrentFunction(objects::PoiseFunction* function) -> void;
    [[nodiscard]] auto getCurrentFunction() const -> objects::PoiseFunction*;

    [[nodiscard]] auto getNativeFunctionHash(std::string_view functionName) const -> std::optional<NativeNameHash>;
    [[nodiscard]] auto nativeFunctionArity(NativeNameHash hash) const -> u8;

    auto emitOp(Op op, usize line) -> void;
    auto emitConstant(Value value) -> void;

    [[nodiscard]] auto run(const scanner::Scanner* const scanner) -> RunResult;

private:
    auto registerNatives() -> void;
    auto registerIntNatives() -> void;

    std::string m_mainFilePath;

    std::vector<OpLine> m_globalOps;
    std::vector<Value> m_globalConstants;

    objects::PoiseFunction* m_currentFunction{nullptr};

    std::hash<std::string_view> m_nativeNameHasher;
    NativeFunctionMap m_nativeFunctionLookup;
};  // class Vm
}   // namespace poise::runtime

#endif  // #ifndef POISE_VM_HPP
