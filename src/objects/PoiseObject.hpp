#ifndef POISE_OBJECT_HPP
#define POISE_OBJECT_HPP

#include "../poise.hpp"

#include <cstddef>
#include <string>

namespace poise::objects
{
    class PoiseFunction;

    class PoiseObject
    {
    public:
        enum class ObjectType
        {
            Function,
        };

        PoiseObject() = default;

        PoiseObject(const PoiseObject&) = delete;
        PoiseObject(PoiseObject&&) = delete;

        virtual ~PoiseObject() = default;

        auto incrementRefCount() -> usize;
        auto decrementRefCount() -> usize;
        [[nodiscard]] auto refCount() const -> usize;

        virtual auto asFunction() -> PoiseFunction*;

        virtual auto print() const -> void = 0;
        virtual auto printLn() const -> void = 0;
        [[nodiscard]] virtual auto toString() const -> std::string = 0;
        [[nodiscard]] virtual auto callable() const -> bool;

    private:
        usize m_refCount{};
    };
}

#endif
