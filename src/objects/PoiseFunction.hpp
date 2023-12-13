#ifndef POISE_FUNCTION_HPP
#define POISE_FUNCTION_HPP

#include <cstdint>
#include <span>
#include <vector>

#include "../Poise.hpp"

#include "PoiseObject.hpp"
#include "../runtime/Op.hpp"
#include "../runtime/Value.hpp"

namespace poise::objects {
class PoiseFunction : public PoiseObject
{
public:
    PoiseFunction(std::string name, std::filesystem::path filePath, usize namespaceHash, u8 arity, bool isExported);
    ~PoiseFunction() override = default;

    [[nodiscard]] auto toString() const noexcept -> std::string override;
    [[nodiscard]] auto typeValue() const noexcept -> const runtime::Value& override;

    auto asFunction() noexcept -> PoiseFunction* override;

    auto emitOp(runtime::Op op, usize line) noexcept -> void;
    auto emitConstant(runtime::Value value) noexcept -> void;
    auto setConstant(runtime::Value value, usize index) noexcept -> void;

    [[nodiscard]] auto opList() const noexcept -> std::span<const runtime::OpLine>;
    [[nodiscard]] auto numOps() const noexcept -> usize;
    [[nodiscard]] auto constantList() const noexcept -> std::span<const runtime::Value>;
    [[nodiscard]] auto numConstants() const noexcept -> usize;

    [[nodiscard]] auto name() const noexcept -> std::string_view;
    [[nodiscard]] auto filePath() const noexcept -> const std::filesystem::path&;
    [[nodiscard]] auto arity() const noexcept -> u8;
    [[nodiscard]] auto nameHash() const noexcept -> usize;
    [[nodiscard]] auto namespaceHash() const noexcept -> usize;
    [[nodiscard]] auto exported() const noexcept -> bool;

    auto lamdaAdded() noexcept -> void;
    [[nodiscard]] auto numLambdas() const noexcept -> u32;
    auto addCapture(runtime::Value value) noexcept -> void;
    [[nodiscard]] auto getCapture(usize index) const noexcept -> const runtime::Value&;

    auto printOps() const -> void;

private:
    std::string m_name;
    std::filesystem::path m_filePath;
    u8 m_arity;
    usize m_nameHash;
    usize m_namespaceHash;
    bool m_isExported;

    u32 m_numLambdas{0};

    std::vector<runtime::OpLine> m_ops;
    std::vector<runtime::Value> m_constants;
    std::vector<runtime::Value> m_captures;
};  // class PoiseFunction
}   // namespace poise::objects

#endif  // #ifndef POISE_FUNCTION_HPP
