//
// Created by ryand on 13/12/2023.
//

#include "PoiseList.hpp"
#include "PoiseRange.hpp"
#include "../PoiseException.hpp"
#include "../PoisePack.hpp"

#include <ranges>

namespace poise::objects::iterables {

PoiseList::PoiseList(runtime::Value value)
{
    switch (value.type()) {
        case runtime::types::Type::String: {
            for (const auto c : value.string()) {
                m_data.emplace_back(std::string(1_uz, c));
            }
            break;
        }
        case runtime::types::Type::List: {
            for (const auto& item : value.object()->asList()->data()) {
                m_data.push_back(item);
            }
            break;
        }
        case runtime::types::Type::Range: {
            m_data = value.object()->asRange()->toVector();
            break;
        }
        case runtime::types::Type::Pack: {
            for (const auto& item : value.object()->asPack()->values()) {
                m_data.push_back(item);
            }
            break;
        }
        default: {
            m_data.emplace_back(std::move(value));
            break;
        }
    }
}

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
    return iterator == end();
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
            if (value.type() == runtime::types::Type::String) {
                res.push_back('"');
                res.append(value.string());
                res.push_back('"');
            } else {
                res.append(value.toString());
            }
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

auto PoiseList::iterable() const noexcept -> bool
{
    return true;
}

auto PoiseList::empty() const noexcept -> bool
{
    return m_data.empty();
}

auto PoiseList::size() const noexcept -> usize
{
    return m_data.size();
}

auto PoiseList::at(usize index) const -> const runtime::Value&
{
    if (index >= size()) {
        throw PoiseException(PoiseException::ExceptionType::IndexOutOfBounds, fmt::format("The index is {} but the size is {}", index, size()));
    }

    return m_data[index];
}

auto PoiseList::at(usize index) -> runtime::Value&
{
    if (index >= size()) {
        throw PoiseException(PoiseException::ExceptionType::IndexOutOfBounds, fmt::format("The index is {} but the size is {}", index, size()));
    }

    return m_data[index];
}

auto PoiseList::append(runtime::Value value) noexcept -> void
{
    m_data.emplace_back(std::move(value));
    invalidateIterators();
}

auto PoiseList::insert(usize index, runtime::Value value) noexcept -> bool
{
    if (index >= m_data.size()) {
        return false;
    }

    m_data.insert(m_data.begin() + static_cast<DifferenceType>(index), std::move(value));
    invalidateIterators();
    return true;
}

auto PoiseList::remove(const runtime::Value& value) noexcept -> i64
{
    const auto res = static_cast<i64>(std::erase(m_data, value));
    if (res > 0) {
        invalidateIterators();
    }
    return res;
}

auto PoiseList::removeFirst(const runtime::Value& value) noexcept -> bool
{
    const auto it = std::find(m_data.cbegin(), m_data.cend(), value);
    if (it == m_data.cend()) {
        return false;
    }

    m_data.erase(it);
    invalidateIterators();
    return true;
}

auto PoiseList::removeAt(usize index) noexcept -> bool
{
    if (index >= m_data.size()) {
        return false;
    }

    m_data.erase(m_data.begin() + static_cast<DifferenceType>(index));
    invalidateIterators();
    return true;
}

auto PoiseList::clear() noexcept -> void
{
    m_data.clear();
    invalidateIterators();
}

auto PoiseList::repeat(isize n) const -> runtime::Value
{
    if (n < 0_iz) {
        throw PoiseException(PoiseException::ExceptionType::ArgumentOutOfRange, fmt::format("Number of repeats must be positive but got {}", n));
    }
    std::vector<runtime::Value> data;
    data.resize(m_data.size() * static_cast<usize>(n));
    for (auto i = 0_iz; i < n; i++) {
        std::copy(m_data.begin(), m_data.end(), std::next(data.begin(), i * std::ssize(m_data)));
    }
    return runtime::Value::createObject<PoiseList>(std::move(data));
}

auto PoiseList::concat(const PoiseList& other) const noexcept -> runtime::Value
{
    std::vector<runtime::Value> data = m_data;
    data.insert(data.end(), other.data().begin(), other.data().end());
    return runtime::Value::createObject<PoiseList>(std::move(data));
}
}   // namespace poise::objects::iterables
