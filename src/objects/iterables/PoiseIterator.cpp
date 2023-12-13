//
// Created by ryand on 13/12/2023.
//

#include "PoiseIterator.hpp"
#include "PoiseIterable.hpp"
#include "../PoiseException.hpp"

#include <fmt/format.h>

namespace poise::objects::iterables {
PoiseIterator::PoiseIterator(PoiseIterable* iterable)
    : m_iterable{iterable}
    , m_isValid{true}
{
    m_iterable->addIterator(this);
    m_iterable->incrementRefCount();
    m_iterator = m_iterable->begin();
}

PoiseIterator::~PoiseIterator()
{
    m_iterable->removeIterator(this);
    if (m_iterable->decrementRefCount() == 0_uz) {
        delete m_iterable;
    }
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
    if (!valid()) {
        throw PoiseException(PoiseException::ExceptionType::InvalidIterator, "Iterator is no longer valid, due to being incremented past the end of the collection or the collection being destroyed");
    }

    m_iterable->incrementIterator(m_iterator);
}

auto PoiseIterator::invalidate() noexcept -> void
{
    m_isValid = false;
}

auto PoiseIterator::isAtEnd() const noexcept -> bool
{
    return m_iterable->isAtEnd(m_iterator);
}

auto PoiseIterator::valid() const noexcept -> bool
{
    return m_isValid;
}

auto PoiseIterator::value() const -> const runtime::Value&
{
    if (!valid()) {
        throw PoiseException(PoiseException::ExceptionType::InvalidIterator, "Iterator is no longer valid, due to being incremented past the end of the collection or the collection being destroyed");
    }

    return *m_iterator;
}
}   // namespace poise::objects::iterables