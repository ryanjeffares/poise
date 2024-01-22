#ifndef POISE_GC_HPP
#define POISE_GC_HPP

#include "../../Poise.hpp"
#include "../../objects/Object.hpp"

namespace poise::runtime::memory {
auto gcTrackObject(objects::Object* object) noexcept -> void;
auto gcStopTrackingObject(objects::Object* object) noexcept -> void;
auto gcFinalise() noexcept -> void;
} // namespace poise::runtime::memory

#endif // #ifndef POISE_GC_HPP

