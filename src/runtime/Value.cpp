#include "Value.hpp"
#include "../objects/PoiseType.hpp"

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
        } else if (type() == Type::Object) {
            m_data.object = other.object();
            object()->incrementRefCount();
        } else {
            m_data = other.m_data;
        }
    }

    Value::Value(Value&& other) noexcept
        : m_data{other.data()}
        , m_type{other.type()}
    {
        if (type() == Type::String || type() == Type::Object) {
            other.makeNone();
        }
    }

    Value& Value::operator=(const Value& other)
    {
        if (this != &other) {
            if (type() == Type::String) {
                delete m_data.string;
            } else if (type() == Type::Object) {
                if (object()->decrementRefCount() == 0zu) {
                    delete m_data.object;
                }
            }

            m_type = other.type();

            if (type() == Type::String) {
                m_data.string = new std::string{other.toString()};
            } else if (type() == Type::Object) {
                m_data.object = other.object();
                object()->incrementRefCount();
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
            } else if (type() == Type::Object) {
                if (object()->decrementRefCount() == 0zu) {
                    delete m_data.object;
                }
            }

            m_type = other.type();
            m_data = other.data();

            if (type() == Type::String || type() == Type::Object) {
                other.makeNone();
            }
        }

        return *this;
    }

    Value::~Value()
    {
        if (type() == Type::String) {
            delete m_data.string;
        } else if (type() == Type::Object) {
            if (object()->decrementRefCount() == 0zu) {
                delete m_data.object;
            }
        }
    }

    auto Value::none() -> Value
    {
        return std::nullptr_t{};
    }

    auto Value::type() const -> Type
    {
        return m_type;
    }

    auto Value::typeValue() const -> const Value&
    {
        if (type() == Type::Object) {
            return object()->typeValue();
        } else {
            return types::s_typeLookup.at(static_cast<types::Type>(type()));
        }
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

    auto Value::toBool() const -> bool
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

    auto Value::toFloat() const -> f64
    {
        switch (type()) {
            case Type::Bool:
                return value<bool>() ? 1.0 : 0.0;
            case Type::Float:
                return value<f64>();
            case Type::Int:
                return static_cast<f64>(value<i64>());
            case Type::String:
                return std::stod(string());
            default:
                throw std::runtime_error(fmt::format("Cannot convert {} to Float", type()));
        }
    }

    auto Value::toInt() const -> i64
    {
        switch (type()) {
            case Type::Bool:
                return value<bool>() ? 1 : 0;
            case Type::Float:
                return static_cast<i64>(value<f64>());
            case Type::Int:
                return value<i64>();
            case Type::String:
                return std::stoi(string());
            default:
                throw std::runtime_error(fmt::format("Cannot convert {} to Float", type()));
        }
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
                return "none";
            case Type::Object:
                return object()->toString();
            case Type::String:
                return string();
            default:
                POISE_UNREACHABLE();
                return "unknown";
        }
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
                return string() + other.toString();
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
        return !toBool();
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
        return toBool() || other.toBool();
    }

    auto Value::operator&&(const Value& other) const -> bool
    {
        return toBool() && other.toBool();
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
        string_view result = "unknown";

        switch (type) {
            case Value::Type::Bool:
                result = "Bool";
                break;
            case Value::Type::Float:
                result = "Float";
                break;
            case Value::Type::Int:
                result = "Int";
                break;
            case Value::Type::Object:
                result = "Object";
                break;
            case Value::Type::None:
                result = "None";
                break;
            case Value::Type::String:
                result = "String";
                break;
        }

        return formatter<string_view>::format(result, context);
    }
}
