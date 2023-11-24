#ifndef POISE_VALUE_HPP
#define POISE_VALUE_HPP

#include "../poise.hpp"
#include "../objects/PoiseObject.hpp"

#include <fmt/format.h>

#include <string>
#include <type_traits>

namespace poise::runtime
{
    template<typename T>
    static constexpr bool IsBool = std::is_same_v<T, bool>;

    template<typename T>
    static constexpr bool IsFloatingPoint = std::is_floating_point_v<T>;

    template<typename T>
    static constexpr bool IsInteger = std::is_same_v<T, i8> || std::is_same_v<T, u8>
                                    || std::is_same_v<T, i16> || std::is_same_v<T, u16>
                                    || std::is_same_v<T, i32> || std::is_same_v<T, u32>
                                    || std::is_same_v<T, i64> || std::is_same_v<T, u64>
                                    || std::is_same_v<T, isize> || std::is_same_v<T, usize>;

    template<typename T>
    static constexpr bool IsString = std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view> || std::is_same_v<T, const char*>;

    template<typename T>
    static constexpr bool IsNone = std::is_same_v<T, std::nullptr_t>;

    template<typename T>
    concept Primitive = IsBool<T> || IsFloatingPoint<T> || IsInteger<T> || IsString<T> || IsNone<T>;

    template<typename T>
    concept Object = std::is_base_of_v<objects::PoiseObject, T>;

    class Value
    {
    public:
        enum class Type
        {
            Bool,
            Float,
            Int,
            Object,
            None,
            String,
        };

        Value();
        Value(const Value& other);
        Value(Value&& other) noexcept;

        template<Primitive T>
        Value(T value)
        {
            if constexpr (IsString<T>) {
                m_type = Type::String;
                m_data.string = new std::string{std::move(value)};
            } else if constexpr (IsNone<T>) {
                m_type = Type::None;
                m_data.none = value;
            } else if constexpr (IsInteger<T>) {
                m_type = Type::Int;
                m_data.integer = static_cast<i64>(value);
            } else if constexpr (IsFloatingPoint<T>) {
                m_type = Type::Float;
                m_data.floating = static_cast<f64>(value);
            } else if constexpr (IsBool<T>) {
                m_type = Type::Bool;
                m_data.boolean = value;
            }
        }

        Value& operator=(const Value& other);
        Value& operator=(Value&& other) noexcept;

        ~Value();

        template<Object T, typename... Args>
        [[nodiscard]] static auto createObject(Args&&... args) -> Value
        {
            Value value;
            value.m_type = Type::Object;
            value.m_data.object = new T(std::forward<Args>(args)...);
            value.object()->incrementRefCount();
            return value;
        }

        template<Primitive T>
        Value& operator=(T value)
        {
            if (type() == Type::String) {
                delete m_data.string;
            } else if (type() == Type::Object) {
                if (object()->decrementRefCount() == 0zu) {
                    delete m_data.object;
                }
            }

            if constexpr (IsString<T>) {
                m_type = Type::String;
                m_data.string = new std::string{std::move(value)};
            } else if constexpr (IsNone<T>) {
                m_type = Type::None;
                m_data.none = value;
            } else if constexpr (IsInteger<T>) {
                m_type = Type::Int;
                m_data.integer = static_cast<i64>(value);
            } else if constexpr (IsFloatingPoint<T>) {
                m_type = Type::Float;
                m_data.floating = static_cast<f64>(value);
            } else if constexpr (IsBool<T>) {
                m_type = Type::Bool;
                m_data.boolean = value;
            }

            return *this;
        }

        template<Primitive T> requires(!std::is_same_v<T, std::string>)
        [[nodiscard]] auto value() const -> T
        {
            if constexpr (std::is_same_v<T, std::nullptr_t>) {
                return m_data.none;
            } else if constexpr (IsInteger<T>) {
                return static_cast<T>(m_data.integer);
            } else if constexpr (IsFloatingPoint<T>) {
                return static_cast<T>(m_data.floating);
            } else if constexpr (IsBool<T>) {
                return m_data.boolean;
            }
        }

        [[nodiscard]] auto string() const -> const std::string&;
        [[nodiscard]] auto object() const -> objects::PoiseObject*;
        [[nodiscard]] auto type() const -> Type;

        auto print() const -> void;
        auto printLn() const -> void;

        [[nodiscard]] auto asBool() const -> bool;
        [[nodiscard]] auto toString() const -> std::string;
        [[nodiscard]] auto callable() const -> bool;

        [[nodiscard]] auto operator|(const Value& other) const -> Value;
        [[nodiscard]] auto operator^(const Value& other) const -> Value;
        [[nodiscard]] auto operator&(const Value& other) const -> Value;
        [[nodiscard]] auto operator<<(const Value& other) const -> Value;
        [[nodiscard]] auto operator>>(const Value& other) const -> Value;
        [[nodiscard]] auto operator+(const Value& other) const -> Value;
        [[nodiscard]] auto operator-(const Value& other) const -> Value;
        [[nodiscard]] auto operator/(const Value& other) const -> Value;
        [[nodiscard]] auto operator*(const Value& other) const -> Value;
        [[nodiscard]] auto operator%(const Value& other) const -> Value;
        
        [[nodiscard]] auto operator!() const -> Value;
        [[nodiscard]] auto operator~() const -> Value;
        [[nodiscard]] auto operator-() const -> Value;
        [[nodiscard]] auto operator+() const -> Value;

        [[nodiscard]] auto operator==(const Value& other) const -> bool;
        [[nodiscard]] auto operator!=(const Value& other) const -> bool;
        [[nodiscard]] auto operator<(const Value& other) const -> bool;
        [[nodiscard]] auto operator<=(const Value& other) const -> bool;
        [[nodiscard]] auto operator>(const Value& other) const -> bool;
        [[nodiscard]] auto operator>=(const Value& other) const -> bool;
        [[nodiscard]] auto operator||(const Value& other) const -> bool;
        [[nodiscard]] auto operator&&(const Value& other) const -> bool;

    private:
        union
        {
            objects::PoiseObject* object;
            std::nullptr_t none;
            std::string* string;
            i64 integer;
            f64 floating;
            bool boolean;
        } m_data;

        Type m_type;

        [[nodiscard]] auto data() const -> decltype(m_data);
        auto makeNone() -> void;
    };
}

namespace fmt
{
    template<>
    struct formatter<poise::runtime::Value> : formatter<string_view>
    {
        [[nodiscard]] auto format(const poise::runtime::Value& value, format_context& context) const -> decltype(context.out());
    };

    template<>
    struct formatter<poise::runtime::Value::Type> : formatter<string_view>
    {
        [[nodiscard]] auto format(poise::runtime::Value::Type type, format_context& context) const -> decltype(context.out());
    };
}
#endif
