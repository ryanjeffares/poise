//
// Created by ryand on 17/12/2023.
//

#ifndef POISE_PACK_HPP
#define POISE_PACK_HPP

#include "PoiseObject.hpp"
#include "../runtime/Value.hpp"

#include <vector>
#include <span>

namespace poise::objects {
class PoisePack : public PoiseObject
{
public:
    explicit PoisePack(std::vector<runtime::Value> values);
    explicit PoisePack(std::span<runtime::Value> values);

    [[nodiscard]] auto asPack() noexcept -> PoisePack* override;

    [[nodiscard]] auto toString() const noexcept -> std::string override;
    [[nodiscard]] auto type() const noexcept -> runtime::types::Type override;

    [[nodiscard]] auto values() const noexcept -> std::span<const runtime::Value>;
    [[nodiscard]] auto values() noexcept -> std::span<runtime::Value>;
    [[nodiscard]] auto size() noexcept -> usize;

private:
    std::vector<runtime::Value> m_values;
};
}

#endif  // #ifndef POISE_PACK_HPP
