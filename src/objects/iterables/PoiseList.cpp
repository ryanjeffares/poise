//
// Created by ryand on 13/12/2023.
//

#include "PoiseList.hpp"

#include <ranges>

namespace poise::objects::iterables {

PoiseList::PoiseList(std::vector<runtime::Value> data) : PoiseIterable{std::move(data)}
{

}

auto PoiseList::begin() noexcept -> IteratorType
{
    return m_data.begin();
}

auto PoiseList::end() noexcept -> IteratorType
{
    return m_data.end();
}

auto PoiseList::incrementIterator(PoiseIterable::IteratorType& iterator) noexcept -> void
{
    iterator++;
}

auto PoiseList::isAtEnd(const PoiseIterable::IteratorType& iterator) noexcept -> bool
{
    return iterator == m_data.end();
}

auto PoiseList::asList() noexcept -> iterables::PoiseList*
{
    return this;
}

auto PoiseList::toString() const noexcept -> std::string
{
    std::string res = "[";

#ifndef __cpp_lib_ranges_enumerate
    for (auto index = 0_uz; const auto& value : m_data) {
#else
    for (const auto [index, value] : m_data | std::views::enumerate) {
#endif
        // TODO - check this recursively
        if (value.object() == this) {
            res.append("...");
        } else {
            res.append(value.toString());
        }

        if (static_cast<usize>(index) < m_data.size() - 1_uz) {
            res.append(", ");
        }

#ifndef __cpp_lib_ranges_enumerate
        index++;
#endif
    }

    res.append("]");
    return res;
}

auto PoiseList::type() const noexcept -> runtime::types::Type
{
    return runtime::types::Type::List;
}

auto PoiseList::append(runtime::Value value) noexcept -> void
{
    m_data.emplace_back(std::move(value));
}

auto PoiseList::insert(usize index, runtime::Value value) noexcept -> bool
{
    if (index >= m_data.size()) {
        return false;
    }

    m_data.insert(m_data.begin() + static_cast<DifferenceType>(index), std::move(value));
    return true;
}

auto PoiseList::remove(const runtime::Value& value) noexcept -> i64
{
    return static_cast<i64>(std::erase(m_data, value));
}

auto PoiseList::removeFirst(const runtime::Value& value) noexcept -> bool
{
    const auto it = std::find(m_data.cbegin(), m_data.cend(), value);
    if (it == m_data.cend()) {
        return false;
    }

    m_data.erase(it);
    return true;
}

auto PoiseList::removeAt(usize index) noexcept -> bool
{
    if (index >= m_data.size()) {
        return false;
    }

    m_data.erase(m_data.begin() + static_cast<DifferenceType>(index));
    return true;
}
}   // namespace poise::objects::iterables