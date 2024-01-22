#include "Gc.hpp"

#include <fmt/core.h>
#include <fmt/format.h>

#include <unordered_set>

namespace poise::runtime::memory {
static std::unordered_set<objects::Object*> s_trackedObjects;

auto gcTrackObject(objects::Object* object) noexcept -> void
{
    POISE_ASSERT(!s_trackedObjects.contains(object), fmt::format("Already tracking object {}", fmt::ptr(object)));
    s_trackedObjects.insert(object);
}

auto gcStopTrackingObject(objects::Object* object) noexcept -> void
{
    if (!object->tracking()) {
        return;
    }

    POISE_ASSERT(s_trackedObjects.contains(object), fmt::format("Not tracking object {}", fmt::ptr(object)));
    s_trackedObjects.erase(object);
}

auto gcFinalise() noexcept -> void
{
    if (!s_trackedObjects.empty()) {
        for (auto object : s_trackedObjects) {
            fmt::print(stderr, "{} {} at {} is still being tracked with {} references\n", object->type(), object->toString(), fmt::ptr(object), object->refCount());
        }
        
        POISE_ASSERT(false, "Objects were still being tracked at shutdown");
    }
}
} // namespace poise::runtime::memory

