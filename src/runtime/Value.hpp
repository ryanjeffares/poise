#ifndef POISE_VALUE_HPP
#define POISE_VALUE_HPP

#include "../Poise.hpp"
#include "../objects/PoiseObject.hpp"

#include <fmt/format.h>

#include <string>
#include <type_traits>

namespace poise::runtime {
template<typename T, typename... Ts>
static constexpr bool IsSameAsAny = (std::is_same_v<T, Ts> || ...);

template<typename T>
static constexpr bool IsBool = std::is_same_v<T, bool>;

template<typename T>
static constexpr bool IsFloatingPoint = std::is_floating_point_v<T>;

template<typename T>
static constexpr bool IsInteger = IsSameAsAny<T, i8, u8, i16, u16, i32, u32, i64, u64, isize, usize>;

template<typename T>
static constexpr bool IsString = IsSameAsAny<T, std::string, std::string_view, const char*>;

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
        // order matches primitive type tokens
        Bool, Float, Int, None, String, Object,
    };

    Value();
    Value(const Value& other);
    Value(Value&& other) noexcept;

    template<Primitive T>
    /* implicit */ Value(T value)
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
    [[nodiscard]] static auto createObject(Args&& ... args) -> Value
    {
        Value value;
        value.m_type = Type::Object;
        value.m_data.object = new T(std::forward<Args>(args)...);
        value.object()->incrementRefCount();
        return value;
    }

    [[nodiscard]] static auto none() -> Value;

    template<Primitive T>
    Value& operator=(T value)
    {
        if (type() == Type::String) {
            delete m_data.string;
        } else if (type() == Type::Object) {
            if (object()->decrementRefCount() == 0_uz) {
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

    template<Primitive T> requires(!IsString<T>)
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

    [[nodiscard]] auto string() const noexcept -> const std::string&;
    [[nodiscard]] auto object() const noexcept -> objects::PoiseObject*;
    [[nodiscard]] auto type() const noexcept -> Type;
    [[nodiscard]] auto typeValue() const -> const Value&;

    auto print(bool err, bool newLine) const -> void;

    [[nodiscard]] auto toBool() const noexcept -> bool;
    [[nodiscard]] auto toFloat() const -> f64;
    [[nodiscard]] auto toInt() const -> i64;
    [[nodiscard]] auto toString() const noexcept -> std::string;

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

    [[nodiscard]] auto operator!() const noexcept -> Value;
    [[nodiscard]] auto operator~() const -> Value;
    [[nodiscard]] auto operator-() const -> Value;
    [[nodiscard]] auto operator+() const -> Value;

    [[nodiscard]] auto operator==(const Value& other) const noexcept -> bool;
    [[nodiscard]] auto operator!=(const Value& other) const noexcept -> bool;
    [[nodiscard]] auto operator<(const Value& other) const -> bool;
    [[nodiscard]] auto operator<=(const Value& other) const -> bool;
    [[nodiscard]] auto operator>(const Value& other) const -> bool;
    [[nodiscard]] auto operator>=(const Value& other) const -> bool;
    [[nodiscard]] auto operator||(const Value& other) const noexcept -> bool;
    [[nodiscard]] auto operator&&(const Value& other) const noexcept -> bool;

private:
    union
    {
        objects::PoiseObject* object;
        std::nullptr_t none;
        std::string* string;
        i64 integer;
        f64 floating;
        bool boolean;
    } m_data{};

    Type m_type;

    [[nodiscard]] auto data() const -> decltype(m_data);
    auto makeNone() -> void;
};  // class Value
}   // namespace poise::runtime

namespace fmt {
template<>
struct formatter<poise::runtime::Value> : formatter<string_view>
{
    [[nodiscard]] auto
    format(const poise::runtime::Value& value, format_context& context) const -> decltype(context.out());
};

template<>
struct formatter<poise::runtime::Value::Type> : formatter<string_view>
{
    [[nodiscard]] auto
    format(poise::runtime::Value::Type type, format_context& context) const -> decltype(context.out());
};
}   // namespace fmt

#endif  // #ifndef POISE_VALUE_HPP
