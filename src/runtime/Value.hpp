#ifndef POISE_VALUE_HPP
#define POISE_VALUE_HPP

#include "../objects/PoiseObject.hpp"

#include <cstddef>
#include <string>
#include <type_traits>

namespace poise::runtime
{
    template<typename T>
    concept BuiltinPoiseType = std::is_integral_v<T>
        || std::is_floating_point_v<T>
        || std::is_same_v<T, std::string>
        || std::is_null_pointer_v<T>;

    template<typename T>
    concept DerivedPoiseObject = std::is_base_of_v<objects::PoiseObject, T>;

    class Value
    {
    public:
        enum class Type
        {
            Object,
            Null,
            String,
        };

        Value();
        Value(const Value& other);
        Value(Value&& other) noexcept;

        template<BuiltinPoiseType T>
        Value(T value)
        {
            if constexpr (std::is_same_v<T, std::string>) {
                m_type = Type::String;
                m_data.string = new std::string{std::move(value)};
            } else if constexpr (std::is_same_v<T, std::nullptr_t>) {
                m_type = Type::Null;
                m_data.null = value;
            }
        }

        Value& operator=(const Value& other);
        Value& operator=(Value&& other) noexcept;

        ~Value();

        template<DerivedPoiseObject T, typename... Args>
        [[nodiscard]] static auto createObject(Args&&... args) -> Value
        {
            Value value;
            value.m_type = Type::Object;
            value.m_data.object = new T(std::forward<Args>(args)...);
            value.object()->incrementRefCount();
            return value;
        }

        template<BuiltinPoiseType T>
        Value& operator=(T value)
        {
            if (type() == Type::String) {
                delete m_data.string;
            }

            if constexpr (std::is_same_v<T, std::string>) {
                m_type = Type::String;
                m_data.string = new std::string{std::move(value)};
            } else if constexpr (std::is_same_v<T, std::nullptr_t>) {
                m_type = Type::Null;
                m_data.null = value;
            }

            return *this;
        }

        template<BuiltinPoiseType T>
        [[nodiscard]] auto value() const -> const T&
        {
            if constexpr (std::is_same_v<T, std::string>) {
                return *m_data.string;
            } else if constexpr (std::is_same_v<T, std::nullptr_t>) {
                return m_data.null;
            } 
        }

        [[nodiscard]] auto object() -> objects::PoiseObject*;
        [[nodiscard]] auto type() const -> Type;

    private:
        union
        {
            objects::PoiseObject* object;
            std::nullptr_t null;
            std::string* string;
        } m_data;

        Type m_type;

        auto data() const -> decltype(m_data);
        auto nullify() -> void;
    };
}

#endif
