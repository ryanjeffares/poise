#include "PoiseTuple.hpp"
#include "../PoiseException.hpp"

namespace poise::objects::iterables {
PoiseTuple::PoiseTuple(std::vector<runtime::Value> data) : PoiseIterable{std::move(data)}
{

}

PoiseTuple::PoiseTuple(runtime::Value key, runtime::Value value)
{
    m_data.emplace_back(std::move(key));
    m_data.emplace_back(std::move(value));
}

auto PoiseTuple::begin() noexcept -> IteratorType 
{
    return m_data.begin();
}

auto PoiseTuple::end() noexcept -> IteratorType
{
    return m_data.end();
}

auto PoiseTuple::incrementIterator(IteratorType& iterator) noexcept -> void
{
    iterator++;
}

auto PoiseTuple::isAtEnd(const IteratorType& iterator) noexcept -> bool
{
    return iterator == end();
}

auto PoiseTuple::size() const noexcept -> usize
{
    return m_data.size();
}

auto PoiseTuple::ssize() const noexcept -> isize
{
    return std::ssize(m_data);
}

auto PoiseTuple::unpack(std::vector<runtime::Value>& stack) const noexcept -> void
{
    for (const auto& value : m_data) {
        stack.push_back(value);
    }

    stack.emplace_back(size());
}

auto PoiseTuple::asIterable() noexcept -> PoiseIterable*
{
    return this;
}

auto PoiseTuple::asTuple() noexcept -> PoiseTuple*
{
    return this;
}

auto PoiseTuple::toString() const noexcept -> std::string
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

auto PoiseTuple::type() const noexcept -> runtime::types::Type
{
    return runtime::types::Type::Tuple;
}

auto PoiseTuple::iterable() const noexcept -> bool
{
    return true;
}

auto PoiseTuple::at(isize index) const -> const runtime::Value&
{
    if (index >= ssize() || index < 0_iz) {
        throw PoiseException{
            PoiseException::ExceptionType::IndexOutOfBounds,
            fmt::format("The index was {} but the size is {}", index, size())
        };
    }

    return m_data[static_cast<usize>(index)];
}

auto PoiseTuple::atMut(isize index) -> runtime::Value&
{
    if (index >= ssize() || index < 0_iz) {
        throw PoiseException{
            PoiseException::ExceptionType::IndexOutOfBounds,
            fmt::format("The index was {} but the size is {}", index, size())
        };
    }

    return m_data[static_cast<usize>(index)];
}
}   // namespace poise::objects::iterables