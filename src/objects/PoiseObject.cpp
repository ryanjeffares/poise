#include "PoiseObject.hpp"

namespace poise::objects
{
    auto PoiseObject::incrementRefCount() -> std::size_t
    {
        return ++m_refCount;
    }

    auto PoiseObject::decrementRefCount() -> std::size_t
    {
        return --m_refCount;
    }

    auto PoiseObject::refCount() const -> std::size_t
    {
        return m_refCount;
    }

    auto PoiseObject::asFunction() -> PoiseFunction*
    {
        return nullptr;
    }
}
