#include "PoiseObject.hpp"

namespace poise::objects {
auto PoiseObject::incrementRefCount() noexcept -> usize
{
    return ++m_refCount;
}

auto PoiseObject::decrementRefCount() noexcept -> usize
{
    return --m_refCount;
}

auto PoiseObject::refCount() const noexcept -> usize
{
    return m_refCount;
}

auto PoiseObject::asException() noexcept -> PoiseException*
{
    return nullptr;
}

auto PoiseObject::asFunction() noexcept -> PoiseFunction*
{
    return nullptr;
}

auto PoiseObject::asType() noexcept -> PoiseType*
{
    return nullptr;
}
}   // namespace poise::objects
