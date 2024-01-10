//
// Created by ryand on 13/12/2023.
//

#include "PoiseList.hpp"
#include "../PoiseException.hpp"
#include "hashables/PoiseDictionary.hpp"
#include "PoiseRange.hpp"

namespace poise::objects::iterables {

List::List(runtime::Value value)
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
        case runtime::types::Type::Dictionary: {
            m_data = value.object()->asDictionary()->toVector();
            break;
        }
        default: {
            m_data.emplace_back(std::move(value));
            break;
        }
    }
}

List::List(std::vector<runtime::Value> data) : Iterable{std::move(data)}
{

}

auto List::begin() noexcept -> IteratorType
{
    return m_data.begin();
}

auto List::end() noexcept -> IteratorType
{
    return m_data.end();
}

auto List::incrementIterator(Iterable::IteratorType& iterator) noexcept -> void
{
    iterator++;
}

auto List::isAtEnd(const Iterable::IteratorType& iterator) noexcept -> bool
{
    return iterator == end();
}

auto List::size() const noexcept -> usize
{
    return m_data.size();
}

auto List::ssize() const noexcept -> isize
{
    return std::ssize(m_data);
}

auto List::unpack(std::vector<runtime::Value> &stack) const noexcept -> void
{
    for (const auto& value : m_data) {
        stack.push_back(value);
    }
    stack.emplace_back(size());
}

auto List::asIterable() noexcept -> Iterable*
{
    return this;
}

auto List::asList() noexcept -> List*
{
    return this;
}

auto List::toString() const noexcept -> std::string
{
    std::string res = "[";

    for (auto index = 0_uz; const auto& value : m_data) {
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

        if (index < m_data.size() - 1_uz) {
            res.append(", ");
        }

        index++;
    }

    res.append("]");
    return res;
}

auto List::type() const noexcept -> runtime::types::Type
{
    return runtime::types::Type::List;
}

auto List::iterable() const noexcept -> bool
{
    return true;
}

auto List::empty() const noexcept -> bool
{
    return m_data.empty();
}

auto List::at(isize index) const -> const runtime::Value&
{
    if (index >= ssize() || index < 0_iz) {
        throw Exception(Exception::ExceptionType::IndexOutOfBounds, fmt::format("The index is {} but the size is {}", index, size()));
    }

    return m_data[static_cast<usize>(index)];
}

auto List::at(isize index) -> runtime::Value&
{
    if (index >= ssize() || index < 0_iz) {
        throw Exception(Exception::ExceptionType::IndexOutOfBounds, fmt::format("The index is {} but the size is {}", index, size()));
    }

    return m_data[static_cast<usize>(index)];
}

auto List::append(runtime::Value value) noexcept -> void
{
    m_data.emplace_back(std::move(value));
    invalidateIterators();
}

auto List::insert(usize index, runtime::Value value) noexcept -> bool
{
    if (index >= m_data.size()) {
        return false;
    }

    m_data.insert(m_data.begin() + static_cast<DifferenceType>(index), std::move(value));
    invalidateIterators();
    return true;
}

auto List::remove(const runtime::Value& value) noexcept -> i64
{
    const auto res = static_cast<i64>(std::erase(m_data, value));
    if (res > 0) {
        invalidateIterators();
    }
    return res;
}

auto List::removeFirst(const runtime::Value& value) noexcept -> bool
{
    const auto it = std::find(m_data.cbegin(), m_data.cend(), value);
    if (it == m_data.cend()) {
        return false;
    }

    m_data.erase(it);
    invalidateIterators();
    return true;
}

auto List::removeAt(usize index) noexcept -> bool
{
    if (index >= m_data.size()) {
        return false;
    }

    m_data.erase(m_data.begin() + static_cast<DifferenceType>(index));
    invalidateIterators();
    return true;
}

auto List::clear() noexcept -> void
{
    m_data.clear();
    invalidateIterators();
}

auto List::repeat(isize n) const -> runtime::Value
{
    if (n < 0_iz) {
        throw Exception(Exception::ExceptionType::ArgumentOutOfRange, fmt::format("Number of repeats must be positive but got {}", n));
    }
    std::vector<runtime::Value> data;
    data.resize(m_data.size() * static_cast<usize>(n));
    for (auto i = 0_iz; i < n; i++) {
        std::copy(m_data.begin(), m_data.end(), std::next(data.begin(), i * std::ssize(m_data)));
    }
    return runtime::Value::createObject<List>(std::move(data));
}

auto List::concat(const List& other) const noexcept -> runtime::Value
{
    std::vector<runtime::Value> data = m_data;
    data.insert(data.end(), other.data().begin(), other.data().end());
    return runtime::Value::createObject<List>(std::move(data));
}
}   // namespace poise::objects::iterables
