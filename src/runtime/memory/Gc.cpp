#include "Gc.hpp"

#include <fmt/core.h>
#include <fmt/format.h>

#include <algorithm>

namespace poise::runtime::memory {
namespace ranges = std::ranges;
using namespace objects;

auto Gc::initialise() noexcept -> void
{
    m_trackedObjects.clear();
    m_roots.clear();
    m_totalAllocatedObjects = 0_uz;
    m_nextCleanCycles = 8_uz;
}

auto Gc::trackObject(Object* object) noexcept -> void
{
    POISE_ASSERT(!ranges::contains(m_trackedObjects, object), fmt::format("Already tracking object {} {} at {}", object->type(), object->toString(), fmt::ptr(object)));
    m_trackedObjects.push_back(object);
    m_totalAllocatedObjects++;
}

auto Gc::stopTrackingObject(Object* object) noexcept -> void
{
    if (!object->tracking()) {
        return;
    }

    POISE_ASSERT(ranges::contains(m_trackedObjects, object), fmt::format("Not tracking {} {} at {}", object->type(), object->toString(), fmt::ptr(object)));

    const auto it = ranges::find(m_trackedObjects, object);
    m_trackedObjects.erase(it);
}

auto Gc::markRoot(Object* root) noexcept -> void
{
    m_roots.push_back(root);
}

auto Gc::finalise() noexcept -> void
{
    cleanCycles();

    if (!m_trackedObjects.empty()) {
        for (auto object : m_trackedObjects) {
            fmt::print(stderr, "{} {} at {} is still being tracked with {} references\n", object->type(), object->toString(), fmt::ptr(object), object->refCount());
        }
        
        POISE_ASSERT(false, "Objects were still being tracked at shutdown");
    }
}

auto Gc::shouldCleanCycles() noexcept -> bool
{
    if (m_totalAllocatedObjects > m_nextCleanCycles) {
        m_nextCleanCycles *= 2_uz;
        return true;
    }

    return false;
}

auto Gc::cleanCycles() noexcept -> void
{
    fmt::print("Cleaning cycles\n");

    // roots have been marked (the stack and local variables)
    // so find every object reachable from these roots
    std::vector<Object*> reachableObjects;
    for (auto object : m_roots) {
        reachableObjects.push_back(object);
        object->findObjectMembers(reachableObjects);
    }

    // anything that's not reachable needs to be deleted
    // give them an extra reference...
    std::vector<Object*> temps;
    for (const auto object : m_trackedObjects) {
        if (!ranges::contains(reachableObjects, object)) {
            object->incrementRefCount();
            temps.push_back(object);
        }
    }

    // ...so we can remove their members and safely delete them
    for (auto object : temps) {
        object->removeObjectMembers();
        stopTrackingObject(object);
        delete object;
    }

    // clear roots for the next call to cleanCycles()
    m_roots.clear();
}
} // namespace poise::runtime::memory

