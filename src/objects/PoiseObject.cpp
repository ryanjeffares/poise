#include "PoiseObject.hpp"

namespace poise::objects {
auto PoiseObject::incrementRefCount() -> usize
{
    return ++m_refCount;
}

auto PoiseObject::decrementRefCount() -> usize
{
    return --m_refCount;
}

auto PoiseObject::refCount() const -> usize
{
    return m_refCount;
}

auto PoiseObject::asFunction() -> PoiseFunction*
{
    return nullptr;
}

auto PoiseObject::asType() -> PoiseType*
{
    return nullptr;
}
}   // namespace poise::objects
