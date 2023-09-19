#ifndef POISE_OBJECT_HPP
#define POISE_OBJECT_HPP

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

        auto incrementRefCount() -> std::size_t;
        auto decrementRefCount() -> std::size_t;
        [[nodiscard]] auto refCount() const -> std::size_t;

        virtual auto asFunction() -> PoiseFunction*;

        virtual auto print() const -> void = 0;
        virtual auto printLn() const -> void = 0;
        [[nodiscard]] virtual auto toString() const -> std::string = 0;

    private:
        std::size_t m_refCount{};
    };
}

#endif
