#ifndef POISE_VM_HPP
#define POISE_VM_HPP

#include "../Poise.hpp"

#include "../objects/PoiseFunction.hpp"
#include "Op.hpp"
#include "../scanner/Scanner.hpp"
#include "Value.hpp"

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

    Vm(std::string mainFilePath);

    auto setCurrentFunction(objects::PoiseFunction* function) -> void;
    [[nodiscard]] auto getCurrentFunction() const -> objects::PoiseFunction*;

    auto emitOp(Op op, usize line) -> void;
    auto emitConstant(Value value) -> void;

    [[nodiscard]] auto run(const scanner::Scanner* const scanner) -> RunResult;

private:
    std::string m_mainFilePath;

    std::vector<OpLine> m_globalOps;
    std::vector<Value> m_globalConstants;

    objects::PoiseFunction* m_currentFunction{nullptr};
};  // class Vm
}   // namespace poise::runtime

#endif  // #ifndef POISE_VM_HPP
