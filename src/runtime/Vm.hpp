#ifndef POISE_VM_HPP
#define POISE_VM_HPP

#include "../Poise.hpp"

#include "../objects/PoiseFunction.hpp"
#include "Op.hpp"
#include "NamespaceManager.hpp"
#include "NativeFunction.hpp"
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

    explicit Vm(std::string mainFilePath);

    auto setCurrentFunction(objects::PoiseFunction* function) noexcept -> void;
    [[nodiscard]] auto currentFunction() const noexcept -> objects::PoiseFunction*;

    [[nodiscard]] auto nativeFunctionHash(std::string_view functionName) const noexcept -> std::optional<NativeNameHash>;
    [[nodiscard]] auto nativeFunctionArity(NativeNameHash hash) const noexcept -> u8;

    [[nodiscard]] auto namespaceManager() const -> const NamespaceManager*;
    [[nodiscard]] auto namespaceManager() -> NamespaceManager*;

    auto emitOp(Op op, usize line) noexcept -> void;
    auto emitConstant(Value value) noexcept -> void;

    [[nodiscard]] auto run(const scanner::Scanner* scanner) noexcept -> RunResult;

private:
    auto registerNatives() noexcept -> void;
    auto registerIntNatives() noexcept -> void;
    auto registerFloatNatives() noexcept -> void;
    auto registerListNatives() noexcept -> void;

    std::hash<std::string_view> m_nativeNameHasher;
    NativeFunctionMap m_nativeFunctionLookup;

    std::string m_mainFilePath;

    std::vector<OpLine> m_globalOps;
    std::vector<Value> m_globalConstants;

    objects::PoiseFunction* m_currentFunction{nullptr};

    NamespaceManager m_namespaceManager;
};  // class Vm
}   // namespace poise::runtime

#endif  // #ifndef POISE_VM_HPP
