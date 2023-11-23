#include "Value.hpp"

#include <fmt/core.h>

namespace poise::runtime
{
    Value::Value()
        : m_type{Type::None}
    {
        m_data.none = std::nullptr_t{};
    }

    Value::Value(const Value& other)
        : m_type{other.m_type}
    {
        if (type() == Type::String) {
            m_data.string = new std::string{other.string()};
        } else {
            m_data = other.m_data;
        }
    }

    Value::Value(Value&& other) noexcept
        : m_data{other.data()}
        , m_type{other.type()}
    {
        if (type() == Type::String) {
            other.makeNone();
        }
    }

    Value& Value::operator=(const Value& other)
    {
        if (this != &other) {
            if (type() == Type::String) {
                delete m_data.string;
            }

            m_type = other.type();
            if (type() == Type::String) {
                m_data.string = new std::string{other.toString()};
            } else {
                m_data = other.data();
            }
        }

        return *this;
    }

    Value& Value::operator=(Value&& other) noexcept
    {
        if (this != &other) {
            if (type() == Type::String) {
                delete m_data.string;
            }

            m_type = other.type();
            m_data = other.data();

            if (type() == Type::String) {
                other.makeNone();
            }
        }

        return *this;
    }

    Value::~Value()
    {
        if (type() == Type::String) {
            delete m_data.string;
        }
    }

    auto Value::type() const -> Type
    {
        return m_type;
    }

    auto Value::string() const -> const std::string&
    {
        return *m_data.string;
    }

    auto Value::object() const -> objects::PoiseObject*
    {
        return m_data.object;
    }

    auto Value::print() const -> void
    {
        fmt::print("{}", toString());
    }

    auto Value::printLn() const -> void
    {
        fmt::print("{}\n", toString());
    }

    auto Value::asBool() const -> bool
    {
        switch (type())
        {
            case Type::Bool:
                return value<bool>();
            case Type::Float:
                return value<f64>() != 0.0;
            case Type::Int:
                return value<i64>() != 0;
            case Type::None:
                return false;
            case Type::Object:
                return true;
            case Type::String:
                return !string().empty();
        }

        return false;
    }

    auto Value::toString() const -> std::string
    {
        switch (type()) {
            case Type::Bool:
                return fmt::format("{}", value<bool>());
            case Type::Float:
                return fmt::format("{}", value<f64>());
            case Type::Int:
                return fmt::format("{}", value<i64>());
            case Type::None:
                return "None";
            case Type::Object:
                return object()->toString();
            case Type::String:
                return string();
            default:
                POISE_UNREACHABLE();
                return "unknown";
        }
    }

    auto Value::callable() const -> bool
    {
        return type() == Type::Object && object()->callable();
    }

    auto Value::operator|(const Value& other) const -> Value
    {
        switch (type()) {
            case Type::Int: {
                switch (other.type()) {
                    case Type::Int:
                        return value<i64>() | other.value<i64>();
                    default:
                        throw std::runtime_error(fmt::format("Invalid operand types for |: '{}' and '{}'", type(), other.type()));
                }
            }
            default:
                throw std::runtime_error(fmt::format("Invalid operand types for |: '{}' and '{}'", type(), other.type()));
        }
    }

    auto Value::operator^(const Value& other) const -> Value
    {
        switch (type()) {
            case Type::Int: {
                switch (other.type()) {
                    case Type::Int:
                        return value<i64>() ^ other.value<i64>();
                    default:
                        throw std::runtime_error(fmt::format("Invalid operand types for ^: '{}' and '{}'", type(), other.type()));
                }
            }
            default:
                throw std::runtime_error(fmt::format("Invalid operand types for ^: '{}' and '{}'", type(), other.type()));
        }
    }

    auto Value::operator&(const Value& other) const -> Value
    {
        switch (type()) {
            case Type::Int: {
                switch (other.type()) {
                    case Type::Int:
                        return value<i64>() & other.value<i64>();
                    default:
                        throw std::runtime_error(fmt::format("Invalid operand types for &: '{}' and '{}'", type(), other.type()));
                }
            }
            default:
                throw std::runtime_error(fmt::format("Invalid operand types for &: '{}' and '{}'", type(), other.type()));
        }
    }

    auto Value::operator<<(const Value& other) const -> Value
    {
        switch (type()) {
            case Type::Int: {
                switch (other.type()) {
                    case Type::Int:
                        return value<i64>() << other.value<i64>();
                    default:
                        throw std::runtime_error(fmt::format("Invalid operand types for <<: '{}' and '{}'", type(), other.type()));
                }
            }
            default:
                throw std::runtime_error(fmt::format("Invalid operand types for <<: '{}' and '{}'", type(), other.type()));
        }
    }

    auto Value::operator>>(const Value& other) const -> Value
    {
        switch (type()) {
            case Type::Int: {
                switch (other.type()) {
                    case Type::Int:
                        return value<i64>() >> other.value<i64>();
                    default:
                        throw std::runtime_error(fmt::format("Invalid operand types for >>: '{}' and '{}'", type(), other.type()));
                }
            }
            default:
                throw std::runtime_error(fmt::format("Invalid operand types for >>: '{}' and '{}'", type(), other.type()));
        }
    }

    auto Value::operator+(const Value& other) const -> Value
    {
        switch (type()) {
            case Type::Float: {
                switch (other.type()) {
                    case Type::Float:
                        return value<f64>() + other.value<f64>();
                    case Type::Int:
                        return value<f64>() + static_cast<f64>(other.value<i64>());
                    default:
                        throw std::runtime_error(fmt::format("Invalid operand types for +: '{}' and '{}'", type(), other.type()));
                }
            }
            case Type::Int: {
                switch (other.type()) {
                    case Type::Float:
                        return static_cast<f64>(value<i64>()) + other.value<f64>();
                    case Type::Int:
                        return value<i64>() + other.value<i64>();
                    default:
                        throw std::runtime_error(fmt::format("Invalid operand types for +: '{}' and '{}'", type(), other.type()));
                }
            }
            case Type::String:
                return string() + other.string();
            default:
                throw std::runtime_error(fmt::format("Invalid operand types for +: '{}' and '{}'", type(), other.type()));
        }
    }

    auto Value::operator-(const Value& other) const -> Value
    {
        switch (type()) {
            case Type::Float: {
                switch (other.type()) {
                    case Type::Float:
                        return value<f64>() - other.value<f64>();
                    case Type::Int:
                        return value<f64>() - static_cast<f64>(other.value<i64>());
                    default:
                        throw std::runtime_error(fmt::format("Invalid operand types for -: '{}' and '{}'", type(), other.type()));
                }
            }
            case Type::Int: {
                switch (other.type()) {
                    case Type::Float:
                        return static_cast<f64>(value<i64>()) - other.value<f64>();
                    case Type::Int:
                        return value<i64>() - other.value<i64>();
                    default:
                        throw std::runtime_error(fmt::format("Invalid operand types for -: '{}' and '{}'", type(), other.type()));
                }
            }
            default:
                throw std::runtime_error(fmt::format("Invalid operand types for -: '{}' and '{}'", type(), other.type()));
        }
    }

    auto Value::operator/(const Value& other) const -> Value
    {
        switch (type()) {
            case Type::Float: {
                switch (other.type()) {
                    case Type::Float:
                        return value<f64>() / other.value<f64>();
                    case Type::Int:
                        return value<f64>() / static_cast<f64>(other.value<i64>());
                    default:
                        throw std::runtime_error(fmt::format("Invalid operand types for /: '{}' and '{}'", type(), other.type()));
                }
            }
            case Type::Int: {
                switch (other.type()) {
                    case Type::Float:
                        return static_cast<f64>(value<i64>()) / other.value<f64>();
                    case Type::Int:
                        return value<i64>() / other.value<i64>();
                    default:
                        throw std::runtime_error(fmt::format("Invalid operand types for /: '{}' and '{}'", type(), other.type()));
                }
            }
            default:
                throw std::runtime_error(fmt::format("Invalid operand types for /: '{}' and '{}'", type(), other.type()));
        }
    }

    auto Value::operator*(const Value& other) const -> Value
    {
        switch (type()) {
            case Type::Float: {
                switch (other.type()) {
                    case Type::Float:
                        return value<f64>() * other.value<f64>();
                    case Type::Int:
                        return value<f64>() * static_cast<f64>(other.value<i64>());
                    default:
                        throw std::runtime_error(fmt::format("Invalid operand types for *: '{}' and '{}'", type(), other.type()));
                }
            }
            case Type::Int: {
                switch (other.type()) {
                    case Type::Float:
                        return static_cast<f64>(value<i64>()) * other.value<f64>();
                    case Type::Int:
                        return value<i64>() * other.value<i64>();
                    default:
                        throw std::runtime_error(fmt::format("Invalid operand types for *: '{}' and '{}'", type(), other.type()));
                }
            }
            case Type::String: {
                switch (other.type()) {
                    case Type::Int: {
                        if (other.value<i64>() < 0) {
                            throw std::runtime_error("Factor to repeat string cannot be negative");
                        }

                        std::string res;
                        res.reserve(string().size() * other.value<usize>());
                        for (auto i = 0zu; i < other.value<usize>(); i++) {
                            res.append(string());
                        }

                        return res;
                    }
                    default:
                        throw std::runtime_error(fmt::format("Invalid operand types for *: '{}' and '{}'", type(), other.type()));
                }
            }
            default:
                throw std::runtime_error(fmt::format("Invalid operand types for *: '{}' and '{}'", type(), other.type()));
        }
    }

    auto Value::operator%(const Value& other) const -> Value
    {
        switch (type()) {
            case Type::Int: {
                switch (other.type()) {
                    case Type::Int: {
                        return value<i64>() % other.value<i64>();
                    }
                    default:
                        throw std::runtime_error(fmt::format("Invalid operand types for %: '{}' and '{}'", type(), other.type()));
                }
            }
            default:
                throw std::runtime_error(fmt::format("Invalid operand types for %: '{}' and '{}'", type(), other.type()));
        }
    }

    auto Value::operator!() const -> Value
    {
        return !asBool();
    }

    auto Value::operator~() const -> Value
    {
        switch (type()) {
            case Type::Int:
                return ~value<i64>();
            default:
                throw std::runtime_error(fmt::format("Invalid operand type for ~: '{}'", type()));
        }
    }

    auto Value::operator-() const -> Value
    {
        switch (type()) {
            case Type::Int:
                return -value<i64>();
            case Type::Float:
                return -value<f64>();
            default:
                throw std::runtime_error(fmt::format("Invalid operand type for -: '{}'", type()));
        }
    }

    auto Value::operator+() const -> Value
    {
        switch (type()) {
            case Type::Int:
                return +value<i64>();
            case Type::Float:
                return +value<f64>();
            default:
                throw std::runtime_error(fmt::format("Invalid operand type for +: '{}'", type()));
        }
    }

    auto Value::operator==(const Value& other) const -> bool
    {
        switch (type()) {
            case Type::Bool: {
                switch (other.type()) {
                    case Type::Bool: {
                        return value<bool>() == other.value<bool>();
                    }
                    default:
                        return false;
                }
            }
            case Type::Float: {
                switch (other.type()) {
                    case Type::Float: {
                        return value<f64>() == other.value<f64>();
                    }
                    case Type::Int: {
                        return value<f64>() == static_cast<f64>(other.value<i64>());
                    }
                    default:
                        return false;
                }
            }
            case Type::Int: {
                switch (other.type()) {
                    case Type::Float: {
                        return value<i64>() == static_cast<i64>(other.value<f64>());
                    }
                    case Type::Int: {
                        return value<i64>() == other.value<i64>();
                    }
                    default:
                        return false;
                }
            }
            case Type::Object: {
                switch (other.type()) {
                    case Type::Object: {
                        return object() == other.object();
                    }
                    default:
                        return false;
                }
            }
            case Type::None: {
                return other.type() == Type::None;
            }
            case Type::String: {
                return other.type() == Type::String && string() == other.string();
            }
            default:
                return false;
        }
    }

    auto Value::operator!=(const Value& other) const -> bool
    {
        return !(*this == other);
    }

    auto Value::operator<(const Value& other) const -> bool
    {
        switch (type()) {
            case Type::Float: {
                switch (other.type()) {
                    case Type::Float: {
                        return value<f64>() < other.value<f64>();
                    }
                    case Type::Int: {
                        return value<f64>() < static_cast<f64>(other.value<i64>());
                    }
                    default:
                        throw std::runtime_error(fmt::format("Invalid operand type for <: '{}' and '{}'", type(), other.type()));
                }
            }
            case Type::Int: {
                switch (other.type()) {
                    case Type::Float: {
                        return value<i64>() < static_cast<i64>(other.value<f64>());
                    }
                    case Type::Int: {
                        return value<i64>() < other.value<i64>();
                    }
                    default:
                        throw std::runtime_error(fmt::format("Invalid operand type for <: '{}' and '{}'", type(), other.type()));
                }
            }
            default:
                throw std::runtime_error(fmt::format("Invalid operand type for <: '{}' and '{}'", type(), other.type()));
        }
    }

    auto Value::operator<=(const Value& other) const -> bool
    {
        switch (type()) {
            case Type::Float: {
                switch (other.type()) {
                    case Type::Float: {
                        return value<f64>() <= other.value<f64>();
                    }
                    case Type::Int: {
                        return value<f64>() <= static_cast<f64>(other.value<i64>());
                    }
                    default:
                        throw std::runtime_error(fmt::format("Invalid operand type for <=: '{}' and '{}'", type(), other.type()));
                }
            }
            case Type::Int: {
                switch (other.type()) {
                    case Type::Float: {
                        return value<i64>() <= static_cast<i64>(other.value<f64>());
                    }
                    case Type::Int: {
                        return value<i64>() <= other.value<i64>();
                    }
                    default:
                        throw std::runtime_error(fmt::format("Invalid operand type for <=: '{}' and '{}'", type(), other.type()));
                }
            }
            default:
                throw std::runtime_error(fmt::format("Invalid operand type for <=: '{}' and '{}'", type(), other.type()));
        }
    }

    auto Value::operator>(const Value& other) const -> bool
    {
        return !(*this <= other);
    }

    auto Value::operator>=(const Value& other) const -> bool
    {
        return !(*this < other);
    }

    auto Value::operator||(const Value& other) const -> bool
    {
        return asBool() || other.asBool();
    }

    auto Value::operator&&(const Value& other) const -> bool
    {
        return asBool() && other.asBool();
    }

    auto Value::data() const -> decltype(m_data)
    {
        return m_data;
    }

    auto Value::makeNone() -> void
    {
        m_data.none = std::nullptr_t{};
        m_type = Type::None;
    }
}

namespace fmt
{
    using namespace poise::runtime;

    auto formatter<Value>::format(const Value& value, format_context& context) const -> decltype(context.out())
    {
        return formatter<string_view>::format(value.toString(), context);
    }

    auto formatter<Value::Type>::format(Value::Type type, format_context& context) const -> decltype(context.out())
    {
        switch (type) {
            case Value::Type::Bool:
                return formatter<string_view>::format("Bool", context);
            case Value::Type::Float:
                return formatter<string_view>::format("Float", context);
            case Value::Type::Int:
                return formatter<string_view>::format("Int", context);
            case Value::Type::Object:
                return formatter<string_view>::format("Object", context);
            case Value::Type::None:
                return formatter<string_view>::format("None", context);
            case Value::Type::String:
                return formatter<string_view>::format("String", context);
            default:
                POISE_UNREACHABLE();
                return formatter<string_view>::format("unknown", context);
        }
    }
}
