#ifndef POISE_FUNCTION_HPP
#define POISE_FUNCTION_HPP

#include "../Poise.hpp"

#include "PoiseObject.hpp"
#include "../runtime/Op.hpp"
#include "../runtime/Value.hpp"

#include <span>
#include <vector>

namespace poise::objects {
class Function : public Object
{
public:
    Function(std::string name, std::filesystem::path filePath, usize namespaceHash, u8 arity, bool isExported, bool hasPack);
    ~Function() override = default;

    [[nodiscard]] auto toString() const noexcept -> std::string override;
    [[nodiscard]] auto type() const noexcept -> runtime::types::Type override;

    auto asFunction() noexcept -> Function* override;

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
    [[nodiscard]] auto hasVariadicParams() const noexcept -> bool;

    auto lamdaAdded() noexcept -> void;
    [[nodiscard]] auto numLambdas() const noexcept -> u32;
    auto addCapture(runtime::Value value) noexcept -> void;
    [[nodiscard]] auto getCapture(usize index) const noexcept -> const runtime::Value&;
    [[nodiscard]] auto shallowClone() const noexcept -> runtime::Value;

    auto printOps() const -> void;

private:
    auto copyData(const Function& other) -> void;

    std::string m_name;
    std::filesystem::path m_filePath;
    u8 m_arity;
    usize m_nameHash;
    usize m_namespaceHash;
    bool m_isExported;
    bool m_hasVariadicParams;

    u32 m_numLambdas{0};

    std::vector<runtime::OpLine> m_ops;
    std::vector<runtime::Value> m_constants;
    std::vector<runtime::Value> m_captures;
};  // class PoiseFunction
}   // namespace poise::objects

#endif  // #ifndef POISE_FUNCTION_HPP
