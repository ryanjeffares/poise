//
// Created by ryand on 13/12/2023.
//

#include "Iterator.hpp"
#include "Iterable.hpp"
#include "../Exception.hpp"

#include <fmt/format.h>

#include <algorithm>

namespace poise::objects::iterables {
Iterator::Iterator(runtime::Value iterable)
    : m_iterableValue{std::move(iterable)}
    , m_iterablePtr{m_iterableValue.object()->asIterable()}
    , m_isValid{true}
{
    // no need to increase reference count on the iterable
    // holding the Value does this
    m_iterablePtr->addIterator(this);
    m_iterator = m_iterablePtr->begin();
}

Iterator::Iterator(Iterable* iterable)
    : m_iterablePtr{iterable}
    , m_isValid{true}
{
    m_iterablePtr->addIterator(this);
    m_iterator = m_iterablePtr->begin();
}

Iterator::~Iterator()
{
    if (m_iterablePtr != nullptr) {
        m_iterablePtr->removeIterator(this);
    }
}

auto Iterator::asIterator() noexcept -> Iterator*
{
    return this;
}

auto Iterator::toString() const noexcept -> std::string
{
    return fmt::format("<iterator instance at {}>", fmt::ptr(this));
}

auto Iterator::type() const noexcept -> runtime::types::Type
{
    return runtime::types::Type::Iterator;
}

auto Iterator::findObjectMembers(std::unordered_set<Object*>& objects) const noexcept -> void
{
    if (auto [it, inserted] = objects.insert(m_iterablePtr); inserted) {
        m_iterablePtr->findObjectMembers(objects);
    }
}

auto Iterator::removeObjectMembers() noexcept -> void
{
    m_iterableValue = runtime::Value::none();
    m_iterablePtr = nullptr;
}

auto Iterator::anyMemberMatchesRecursive(const Object* object) const noexcept -> bool
{
    if (m_iterablePtr == nullptr) {
        return object == this;
    }

    return object == this || object == m_iterablePtr || m_iterablePtr->anyMemberMatchesRecursive(object);
}

auto Iterator::increment() -> void
{
    throwIfInvalid();
    m_iterablePtr->incrementIterator(m_iterator);
}

auto Iterator::invalidate() noexcept -> void
{
    m_isValid = false;
}

auto Iterator::isAtEnd() const noexcept -> bool
{
    return m_iterablePtr->isAtEnd(m_iterator);
}

auto Iterator::valid() const noexcept -> bool
{
    return m_isValid;
}

auto Iterator::value() const -> const runtime::Value&
{
    throwIfInvalid();
    return *m_iterator;
}

auto Iterator::iterator() const noexcept -> const IteratorType&
{
    return m_iterator;
}

auto Iterator::iterator() noexcept -> IteratorType&
{
    return m_iterator;
}

auto Iterator::throwIfInvalid() const -> void
{
    if (!valid()) {
        throw Exception(Exception::ExceptionType::InvalidIterator, "Iterator is no longer valid due to the collection being modified or destroyed");
    }

    if (isAtEnd()) {
        throw Exception(Exception::ExceptionType::IteratorOutOfBounds, "Iterator has already been incremented past the end of the collection");
    }
}

auto Iterator::iterableValue() const noexcept -> const runtime::Value&
{
    return m_iterableValue;
}

auto Iterator::iterablePtr() const noexcept -> Iterable*
{
    return m_iterablePtr;
}
}   // namespace poise::objects::iterables
