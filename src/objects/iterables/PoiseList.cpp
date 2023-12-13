//
// Created by ryand on 13/12/2023.
//

#include "PoiseList.hpp"

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

auto PoiseList::append(runtime::Value value) noexcept -> void
{
    m_data.emplace_back(std::move(value));
}

auto PoiseList::insert(runtime::Value value, usize index) noexcept -> bool
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