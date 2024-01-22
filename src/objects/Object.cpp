#include "Object.hpp"

namespace poise::objects {
auto Object::incrementRefCount() noexcept -> usize
{
    return ++m_refCount;
}

auto Object::decrementRefCount() noexcept -> usize
{
    return --m_refCount;
}

auto Object::refCount() const noexcept -> usize
{
    return m_refCount;
}

auto Object::tracking() const noexcept -> bool
{
    return m_tracking;
}

auto Object::setTracking(bool tracking) noexcept -> void
{
    m_tracking = tracking;
}

auto Object::asIterable() noexcept -> iterables::Iterable*
{
    return nullptr;
}

auto Object::asHashable() noexcept -> iterables::hashables::Hashable*
{
    return nullptr;
}

auto Object::asDictionary() noexcept -> iterables::hashables::Dict*
{
    return nullptr;
}

auto Object::asException() noexcept -> Exception*
{
    return nullptr;
}

auto Object::asFunction() noexcept -> Function*
{
    return nullptr;
}

auto Object::asIterator() noexcept -> iterables::Iterator*
{
    return nullptr;
}

auto Object::asList() noexcept -> iterables::List*
{
    return nullptr;
}

auto Object::asRange() noexcept -> iterables::Range*
{
    return nullptr;
}

auto Object::asSet() noexcept -> iterables::hashables::Set*
{
    return nullptr;
}

auto Object::asTuple() noexcept -> iterables::Tuple*
{
    return nullptr;
}

auto Object::asType() noexcept -> Type*
{
    return nullptr;
}

auto Object::iterable() const -> bool
{
    return false;
}
}   // namespace poise::objects
