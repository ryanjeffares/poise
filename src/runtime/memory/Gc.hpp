#ifndef POISE_GC_HPP
#define POISE_GC_HPP

#include "../../Poise.hpp"
#include "../../objects/Object.hpp"

#include <vector>

namespace poise::runtime::memory {
class Gc
{
public:
    [[nodiscard]] static auto instance() noexcept -> Gc&
    {
        static auto gc = Gc{};
        return gc;
    }

    Gc(const Gc&) = delete;
    Gc& operator=(const Gc&) = delete;

    auto initialise() noexcept -> void;

    auto trackObject(objects::Object* object) noexcept -> void;
    auto stopTrackingObject(objects::Object* object) noexcept -> void;
    auto markRoot(objects::Object* root) noexcept -> void;
    auto finalise() noexcept -> void;

    [[nodiscard]] auto numTrackedObjects() const noexcept -> usize;
    [[nodiscard]] auto shouldCleanCycles() const noexcept -> bool;
    auto cleanCycles() noexcept -> void;

private:
    Gc() = default;

    usize m_totalAllocatedObjects = 0_uz;
    usize m_nextCleanCycles = 8_uz;

    std::vector<objects::Object*> m_roots;
    std::vector<objects::Object*> m_trackedObjects;
};
} // namespace poise::runtime::memory


#endif // #ifndef POISE_GC_HPP

