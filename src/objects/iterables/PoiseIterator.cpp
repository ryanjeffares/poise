//
// Created by ryand on 13/12/2023.
//

#include "PoiseIterator.hpp"
#include "PoiseIterable.hpp"
#include "../PoiseException.hpp"

#include <fmt/format.h>

namespace poise::objects::iterables {
PoiseIterator::PoiseIterator(runtime::Value iterable)
    : m_iterableValue{std::move(iterable)}
    , m_iterablePtr{m_iterableValue.object()->asIterable()}
    , m_isValid{true}
{
    m_iterableValue.object()->incrementRefCount();
    m_iterablePtr->addIterator(this);
    m_iterator = m_iterablePtr->begin();
}

PoiseIterator::PoiseIterator(PoiseIterable* iterable)
    : m_iterablePtr{iterable}
    , m_isValid{true}
{
    m_iterablePtr->addIterator(this);
    m_iterator = m_iterablePtr->begin();
}

PoiseIterator::~PoiseIterator()
{
    m_iterablePtr->removeIterator(this);
}

auto PoiseIterator::asIterator() noexcept -> PoiseIterator*
{
    return this;
}

auto PoiseIterator::toString() const noexcept -> std::string
{
    return fmt::format("<iterator instance at {}>", fmt::ptr(this));
}

auto PoiseIterator::type() const noexcept -> runtime::types::Type
{
    return runtime::types::Type::Iterator;
}

auto PoiseIterator::increment() -> void
{
    throwIfInvalid();
    m_iterablePtr->incrementIterator(m_iterator);
}

auto PoiseIterator::invalidate() noexcept -> void
{
    m_isValid = false;
}

auto PoiseIterator::isAtEnd() const noexcept -> bool
{
    return m_iterablePtr->isAtEnd(m_iterator);
}

auto PoiseIterator::valid() const noexcept -> bool
{
    return m_isValid;
}

auto PoiseIterator::value() const -> const runtime::Value&
{
    throwIfInvalid();
    return *m_iterator;
}

auto PoiseIterator::iterator() const noexcept -> const IteratorType&
{
    return m_iterator;
}

auto PoiseIterator::iterator() noexcept -> PoiseIterator::IteratorType&
{
    return m_iterator;
}

auto PoiseIterator::throwIfInvalid() const -> void
{
    if (!valid()) {
        throw PoiseException(PoiseException::ExceptionType::InvalidIterator, "Iterator is no longer valid due to the collection being modified or destroyed");
    }

    if (isAtEnd()) {
        throw PoiseException(PoiseException::ExceptionType::IteratorOutOfBounds, "Iterator has already been incremented past the end of the collection");
    }
}

auto PoiseIterator::iterableValue() const noexcept -> const runtime::Value&
{
    return m_iterableValue;
}

auto PoiseIterator::iterablePtr() const noexcept -> PoiseIterable*
{
    return m_iterablePtr;
}
}   // namespace poise::objects::iterables
