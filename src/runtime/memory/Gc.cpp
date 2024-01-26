#include "Gc.hpp"

#include <fmt/core.h>
#include <fmt/format.h>

#include <algorithm>
#ifdef POISE_DEBUG
#include <chrono>
#endif

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
#ifdef __cpp_lib_ranges_contains
    POISE_ASSERT(!ranges::contains(m_trackedObjects, object), fmt::format("Already tracking object {} {} at {}", object->type(), object->toString(), fmt::ptr(object)));
#else
    POISE_ASSERT(std::find(m_trackedObjects.begin(), m_trackedObjects.end(), object) == m_trackedObjects.end(), fmt::format("Already tracking object {} {} at {}", object->type(), object->toString(), fmt::ptr(object)));
#endif

    m_trackedObjects.push_back(object);
    m_totalAllocatedObjects++;
}

auto Gc::stopTrackingObject(Object* object) noexcept -> void
{
    if (!object->tracking()) {
        return;
    }

#ifdef __cpp_lib_ranges_contains
    POISE_ASSERT(ranges::contains(m_trackedObjects, object), fmt::format("Not tracking {} {} at {}", object->type(), object->toString(), fmt::ptr(object)));
#else
    POISE_ASSERT(std::find(m_trackedObjects.begin(), m_trackedObjects.end(), object) != m_trackedObjects.end(), fmt::format("Not tracking {} {} at {}", object->type(), object->toString(), fmt::ptr(object)));
#endif

    const auto it = ranges::find(m_trackedObjects, object);
    m_trackedObjects.erase(it);
}

auto Gc::markRoot(Object* root) noexcept -> void
{
#ifdef __cpp_lib_ranges_contains
    if (!ranges::contains(m_roots, root)) {
#else
    if (std::find(m_roots.begin(), m_roots.end(), root) == m_roots.end()) {
#endif
        m_roots.push_back(root);
    }
}

auto Gc::finalise() noexcept -> void
{
    cleanCycles();

#ifdef POISE_DEBUG
    if (!m_trackedObjects.empty()) {
        for (auto object : m_trackedObjects) {
            fmt::print(stderr, "{} {} at {} is still being tracked with {} references\n", object->type(), object->toString(), fmt::ptr(object), object->refCount());
        }
        
        POISE_ASSERT(false, "Objects were still being tracked at shutdown");
    }
#endif
}

auto Gc::numTrackedObjects() const noexcept -> usize
{
    return m_trackedObjects.size();
}

auto Gc::shouldCleanCycles() const noexcept -> bool
{
    return m_totalAllocatedObjects > m_nextCleanCycles;
}

auto Gc::cleanCycles() noexcept -> void
{
#ifdef POISE_DEBUG
    fmt::print("CLEANING CYCLES\n");
    const auto start = std::chrono::steady_clock::now();
    auto numObjectsDeleted = 0_uz;
#endif

    m_nextCleanCycles *= 2_uz;

    // roots have been marked (the stack, iterators, and local variables)
    // so find every object reachable from these roots
    std::vector<Object*> reachableObjects;
    for (auto object : m_roots) {
#ifdef __cpp_lib_ranges_contains
        if (!ranges::contains(reachableObjects, object)) {
#else
        if (std::find(reachableObjects.begin(), reachableObjects.end(), object) == reachableObjects.end()) {
#endif
            reachableObjects.push_back(object);
            object->findObjectMembers(reachableObjects);
        }
    }

    // anything that's not reachable needs to be deleted
    std::vector<Object*> unreachableObjects;
    for (const auto object : m_trackedObjects) {
#ifdef __cpp_lib_ranges_contains
        if (!ranges::contains(reachableObjects, object)) {
#else
        if (std::find(reachableObjects.begin(), reachableObjects.end(), object) == reachableObjects.end()) {
#endif
            // give them an extra reference to make sure they don't get deleted indirectly
            object->incrementRefCount();
            unreachableObjects.push_back(object);
        }
    }

    for (const auto object : unreachableObjects) {
        // disable tracking and remove them from our lists here
        stopTrackingObject(object);
        object->setTracking(false);
        // remove their members to avoid indirect deletions of deleted objects
        object->removeObjectMembers();
    }

    // and safely delete them
    for (const auto object : unreachableObjects) {
#ifdef POISE_DEBUG
        fmt::print("Deleting unreachable {} at {}\n", object->type(), fmt::ptr(object));
        numObjectsDeleted++;
#endif
        delete object;
    }

    // clear roots for the next call to cleanCycles()
    m_roots.clear();

#ifdef POISE_DEBUG
    const auto end = std::chrono::steady_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    fmt::print("Deleted {} objects in {} ms\n", numObjectsDeleted, duration);
#endif
}
} // namespace poise::runtime::memory
