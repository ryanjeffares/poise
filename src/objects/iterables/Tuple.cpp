#include "Tuple.hpp"
#include "../Exception.hpp"

namespace poise::objects::iterables {
Tuple::Tuple(std::vector<runtime::Value> data) : Iterable{std::move(data)}
{

}

Tuple::Tuple(runtime::Value key, runtime::Value value)
{
    m_data.emplace_back(std::move(key));
    m_data.emplace_back(std::move(value));
}

auto Tuple::begin() noexcept -> IteratorType 
{
    return m_data.begin();
}

auto Tuple::end() noexcept -> IteratorType
{
    return m_data.end();
}

auto Tuple::incrementIterator(IteratorType& iterator) noexcept -> void
{
    iterator++;
}

auto Tuple::isAtEnd(const IteratorType& iterator) noexcept -> bool
{
    return iterator == end();
}

auto Tuple::size() const noexcept -> usize
{
    return m_data.size();
}

auto Tuple::ssize() const noexcept -> isize
{
    return std::ssize(m_data);
}

auto Tuple::unpack(std::vector<runtime::Value>& stack) const noexcept -> void
{
    for (const auto& value : m_data) {
        stack.push_back(value);
    }

    stack.emplace_back(size());
}

auto Tuple::asIterable() noexcept -> Iterable*
{
    return this;
}

auto Tuple::asTuple() noexcept -> Tuple*
{
    return this;
}

auto Tuple::toString() const noexcept -> std::string
{
    std::string res = "(";

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

    res.append(")");
    return res;
}

auto Tuple::type() const noexcept -> runtime::types::Type
{
    return runtime::types::Type::Tuple;
}

auto Tuple::iterable() const noexcept -> bool
{
    return true;
}

auto Tuple::at(isize index) const -> const runtime::Value&
{
    if (index >= ssize() || index < 0_iz) {
        throw Exception{
            Exception::ExceptionType::IndexOutOfBounds,
            fmt::format("The index was {} but the size is {}", index, size())
        };
    }

    return m_data[static_cast<usize>(index)];
}

auto Tuple::atMut(isize index) -> runtime::Value&
{
    if (index >= ssize() || index < 0_iz) {
        throw Exception{
            Exception::ExceptionType::IndexOutOfBounds,
            fmt::format("The index was {} but the size is {}", index, size())
        };
    }

    return m_data[static_cast<usize>(index)];
}
}   // namespace poise::objects::iterables
