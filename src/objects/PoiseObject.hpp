#ifndef POISE_OBJECT_HPP
#define POISE_OBJECT_HPP

#include "../Poise.hpp"

#include <cstddef>
#include <string>

namespace poise::runtime {
class Value;
}

namespace poise::objects {
class PoiseException;
class PoiseFunction;
class PoiseType;

class PoiseObject
{
public:
    enum class ObjectType
    {
        Exception, Function, Type,
    };

    PoiseObject() = default;

    PoiseObject(const PoiseObject&) = delete;
    PoiseObject(PoiseObject&&) = delete;

    virtual ~PoiseObject() = default;

    auto incrementRefCount() noexcept -> usize;
    [[nodiscard]] auto decrementRefCount() noexcept -> usize;
    [[nodiscard]] auto refCount() const noexcept -> usize;

    [[nodiscard]] virtual auto asException() noexcept -> PoiseException*;
    [[nodiscard]] virtual auto asFunction() noexcept -> PoiseFunction*;
    [[nodiscard]] virtual auto asType() noexcept -> PoiseType*;

    virtual auto print() const -> void = 0;
    virtual auto printLn() const -> void = 0;
    [[nodiscard]] virtual auto toString() const noexcept -> std::string = 0;
    [[nodiscard]] virtual auto typeValue() const noexcept -> const runtime::Value& = 0;
    [[nodiscard]] virtual auto objectType() const noexcept -> ObjectType = 0;

private:
    usize m_refCount{};
};  // class PoiseObjects
}   // namespace poise::objects

#endif  // #ifndef POISE_OBJECT_HPP
