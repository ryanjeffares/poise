#ifndef POISE_OBJECT_HPP
#define POISE_OBJECT_HPP

#include "../Poise.hpp"
#include "../runtime/Types.hpp"

#include <cstddef>
#include <string>

namespace poise::runtime {
class Value;
}

namespace poise::objects {
class PoiseException;
class PoiseFunction;
class PoiseType;

namespace iterables {
class PoiseIterator;
class PoiseList;
class PoiseRange;
}

class PoiseObject
{
public:
    PoiseObject() = default;

    PoiseObject(const PoiseObject&) = delete;
    PoiseObject(PoiseObject&&) = delete;

    virtual ~PoiseObject() = default;

    auto incrementRefCount() noexcept -> usize;
    [[nodiscard]] auto decrementRefCount() noexcept -> usize;
    [[nodiscard]] auto refCount() const noexcept -> usize;

    [[nodiscard]] virtual auto asException() noexcept -> PoiseException*;
    [[nodiscard]] virtual auto asFunction() noexcept -> PoiseFunction*;
    [[nodiscard]] virtual auto asIterator() noexcept -> iterables::PoiseIterator*;
    [[nodiscard]] virtual auto asList() noexcept -> iterables::PoiseList*;
    [[nodiscard]] virtual auto asRange() noexcept -> iterables::PoiseRange*;
    [[nodiscard]] virtual auto asType() noexcept -> PoiseType*;

    [[nodiscard]] virtual auto toString() const noexcept -> std::string = 0;
    [[nodiscard]] virtual auto type() const noexcept -> runtime::types::Type = 0;
    [[nodiscard]] virtual auto iterable() const -> bool;

private:
    usize m_refCount{};
};  // class PoiseObjects
}   // namespace poise::objects

#endif  // #ifndef POISE_OBJECT_HPP
