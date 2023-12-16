#include "Value.hpp"
#include "../objects/PoiseType.hpp"
#include "../objects/PoiseException.hpp"

#include <fmt/core.h>

namespace poise::runtime {
Value::Value()
    : m_type{TypeInternal::None}
{
    m_data.none = std::nullptr_t{};
}

Value::Value(const Value& other)
    : m_type{other.typeInternal()}
{
    if (typeInternal() == TypeInternal::String) {
        m_data.string = new std::string{other.string()};
    } else if (typeInternal() == TypeInternal::Object) {
        m_data.object = other.object();
        object()->incrementRefCount();
    } else {
        m_data = other.m_data;
    }
}

Value::Value(Value&& other) noexcept
    : m_data{other.data()}
    , m_type{other.typeInternal()}
{
    if (typeInternal() == TypeInternal::String || typeInternal() == TypeInternal::Object) {
        other.makeNone();
    }
}

Value& Value::operator=(const Value& other)
{
    if (this != &other) {
        if (typeInternal() == TypeInternal::String) {
            delete m_data.string;
        } else if (typeInternal() == TypeInternal::Object) {
            if (object()->decrementRefCount() == 0_uz) {
                delete m_data.object;
            }
        }

        m_type = other.typeInternal();

        if (typeInternal() == TypeInternal::String) {
            m_data.string = new std::string{other.toString()};
        } else if (typeInternal() == TypeInternal::Object) {
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
        if (typeInternal() == TypeInternal::String) {
            delete m_data.string;
        } else if (typeInternal() == TypeInternal::Object) {
            if (object()->decrementRefCount() == 0_uz) {
                delete m_data.object;
            }
        }

        m_type = other.typeInternal();
        m_data = other.data();

        if (typeInternal() == TypeInternal::String || typeInternal() == TypeInternal::Object) {
            other.makeNone();
        }
    }

    return *this;
}

Value::~Value()
{
    if (typeInternal() == TypeInternal::String) {
        delete m_data.string;
    } else if (typeInternal() == TypeInternal::Object) {
        if (object()->decrementRefCount() == 0_uz) {
            delete m_data.object;
        }
    }
}

auto Value::none() -> Value
{
    return std::nullptr_t{};
}

auto Value::string() const noexcept -> const std::string&
{
    return *m_data.string;
}

auto Value::object() const noexcept -> objects::PoiseObject*
{
    return typeInternal() == TypeInternal::Object ? m_data.object : nullptr;
}

auto Value::type() const noexcept -> types::Type
{
    if (typeInternal() == TypeInternal::Object) {
        return object()->type();
    } else {
        return static_cast<types::Type>(typeInternal());
    }
}

auto Value::typeInternal() const noexcept -> TypeInternal
{
    return m_type;
}

auto Value::typeValue() const noexcept -> const Value&
{
    if (typeInternal() == TypeInternal::Object) {
        return types::typeValue(object()->type());
    } else {
        return types::typeValue(static_cast<types::Type>(typeInternal()));
    }
}

auto Value::isNumber() const noexcept -> bool
{
    return typeInternal() == TypeInternal::Int || typeInternal() == TypeInternal::Float;
}

auto Value::print(bool err, bool newLine) const -> void
{
    if (newLine) {
        fmt::print(err ? stderr : stdout, "{}\n", toString());
    } else {
        fmt::print(err ? stderr : stdout, "{}", toString());
    }
}

auto Value::toBool() const noexcept -> bool
{
    switch (typeInternal()) {
        case TypeInternal::Bool:
            return value<bool>();
        case TypeInternal::Float:
            return value<f64>() != 0.0;
        case TypeInternal::Int:
            return value<i64>() != 0;
        case TypeInternal::None:
            return false;
        case TypeInternal::Object:
            return true;
        case TypeInternal::String:
            return !string().empty();
        default:
            POISE_UNREACHABLE();
            return false;
    }
}

auto Value::toFloat() const -> f64
{
    switch (typeInternal()) {
        case TypeInternal::Bool:
            return value<bool>() ? 1.0 : 0.0;
        case TypeInternal::Float:
            return value<f64>();
        case TypeInternal::Int:
            return static_cast<f64>(value<i64>());
        case TypeInternal::String:
            return std::stod(string());
        default:
            throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidType, fmt::format("Cannot convert {} to Float", type()));
    }
}

auto Value::toInt() const -> i64
{
    switch (typeInternal()) {
        case TypeInternal::Bool:
            return value<bool>() ? 1 : 0;
        case TypeInternal::Float:
            return static_cast<i64>(value<f64>());
        case TypeInternal::Int:
            return value<i64>();
        case TypeInternal::String:
            return std::stoi(string());
        default:
            throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidType, fmt::format("Cannot convert {} to Int", type()));
    }
}

auto Value::toString() const noexcept -> std::string
{
    switch (typeInternal()) {
        case TypeInternal::Bool:
            return fmt::format("{}", value<bool>());
        case TypeInternal::Float:
            return fmt::format("{}", value<f64>());
        case TypeInternal::Int:
            return fmt::format("{}", value<i64>());
        case TypeInternal::None:
            return "none";
        case TypeInternal::Object:
            return object()->toString();
        case TypeInternal::String:
            return string();
        default:
            POISE_UNREACHABLE();
            return "unknown";
    }
}

auto Value::operator|(const Value& other) const -> Value
{
    switch (typeInternal()) {
        case TypeInternal::Int: {
            switch (other.typeInternal()) {
                case TypeInternal::Int:
                    return value<i64>() | other.value<i64>();
                default:
                    throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for |: '{}' and '{}'", type(), other.type()));
            }
        }
        default:
            throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for |: '{}' and '{}'", type(), other.type()));
    }
}

auto Value::operator^(const Value& other) const -> Value
{
    switch (typeInternal()) {
        case TypeInternal::Int: {
            switch (other.typeInternal()) {
                case TypeInternal::Int:
                    return value<i64>() ^ other.value<i64>();
                default:
                    throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for ^: '{}' and '{}'", type(), other.type()));
            }
        }
        default:
            throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for ^: '{}' and '{}'", type(), other.type()));
    }
}

auto Value::operator&(const Value& other) const -> Value
{
    switch (typeInternal()) {
        case TypeInternal::Int: {
            switch (other.typeInternal()) {
                case TypeInternal::Int:
                    return value<i64>() & other.value<i64>();
                default:
                    throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for &: '{}' and '{}'", type(), other.type()));
            }
        }
        default:
            throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for &: '{}' and '{}'", type(), other.type()));
    }
}

auto Value::operator<<(const Value& other) const -> Value
{
    switch (typeInternal()) {
        case TypeInternal::Int: {
            switch (other.typeInternal()) {
                case TypeInternal::Int:
                    return value<i64>() << other.value<i64>();
                default:
                    throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for <<: '{}' and '{}'", type(), other.type()));
            }
        }
        default:
            throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for <<: '{}' and '{}'", type(), other.type()));
    }
}

auto Value::operator>>(const Value& other) const -> Value
{
    switch (typeInternal()) {
        case TypeInternal::Int: {
            switch (other.typeInternal()) {
                case TypeInternal::Int:
                    return value<i64>() >> other.value<i64>();
                default:
                    throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for >>: '{}' and '{}'", type(), other.type()));
            }
        }
        default:
            throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for >>: '{}' and '{}'", type(), other.type()));
    }
}

auto Value::operator+(const Value& other) const -> Value
{
    switch (typeInternal()) {
        case TypeInternal::Float: {
            switch (other.typeInternal()) {
                case TypeInternal::Float:
                    return value<f64>() + other.value<f64>();
                case TypeInternal::Int:
                    return value<f64>() + static_cast<f64>(other.value<i64>());
                default:
                    throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for +: '{}' and '{}'", type(), other.type()));
            }
        }
        case TypeInternal::Int: {
            switch (other.typeInternal()) {
                case TypeInternal::Float:
                    return static_cast<f64>(value<i64>()) + other.value<f64>();
                case TypeInternal::Int:
                    return value<i64>() + other.value<i64>();
                default:
                    throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for +: '{}' and '{}'", type(), other.type()));
            }
        }
        case TypeInternal::String:
            return string() + other.toString();
        default:
            throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for +: '{}' and '{}'", type(), other.type()));
    }
}

auto Value::operator-(const Value& other) const -> Value
{
    switch (typeInternal()) {
        case TypeInternal::Float: {
            switch (other.typeInternal()) {
                case TypeInternal::Float:
                    return value<f64>() - other.value<f64>();
                case TypeInternal::Int:
                    return value<f64>() - static_cast<f64>(other.value<i64>());
                default:
                    throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for -: '{}' and '{}'", type(), other.type()));
            }
        }
        case TypeInternal::Int: {
            switch (other.typeInternal()) {
                case TypeInternal::Float:
                    return static_cast<f64>(value<i64>()) - other.value<f64>();
                case TypeInternal::Int:
                    return value<i64>() - other.value<i64>();
                default:
                    throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for -: '{}' and '{}'", type(), other.type()));
            }
        }
        default:
            throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for -: '{}' and '{}'", type(), other.type()));
    }
}

auto Value::operator/(const Value& other) const -> Value
{
    switch (typeInternal()) {
        case TypeInternal::Float: {
            switch (other.typeInternal()) {
                case TypeInternal::Float:
                    return value<f64>() / other.value<f64>();
                case TypeInternal::Int:
                    return value<f64>() / static_cast<f64>(other.value<i64>());
                default:
                    throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for /: '{}' and '{}'", type(), other.type()));
            }
        }
        case TypeInternal::Int: {
            switch (other.typeInternal()) {
                case TypeInternal::Float:
                    return static_cast<f64>(value<i64>()) / other.value<f64>();
                case TypeInternal::Int:
                    return value<i64>() / other.value<i64>();
                default:
                    throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for /: '{}' and '{}'", type(), other.type()));
            }
        }
        default:
            throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for /: '{}' and '{}'", type(), other.type()));
    }
}

auto Value::operator*(const Value& other) const -> Value
{
    switch (typeInternal()) {
        case TypeInternal::Float: {
            switch (other.typeInternal()) {
                case TypeInternal::Float:
                    return value<f64>() * other.value<f64>();
                case TypeInternal::Int:
                    return value<f64>() * static_cast<f64>(other.value<i64>());
                default:
                    throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for *: '{}' and '{}'", type(), other.type()));
            }
        }
        case TypeInternal::Int: {
            switch (other.typeInternal()) {
                case TypeInternal::Float:
                    return static_cast<f64>(value<i64>()) * other.value<f64>();
                case TypeInternal::Int:
                    return value<i64>() * other.value<i64>();
                default:
                    throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for *: '{}' and '{}'", type(), other.type()));
            }
        }
        case TypeInternal::String: {
            switch (other.typeInternal()) {
                case TypeInternal::Int: {
                    if (other.value<i64>() < 0) {
                        throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, "Factor to repeat string cannot be type");
                    }

                    std::string res;
                    res.reserve(string().size() * other.value<usize>());
                    for (auto i = 0_uz; i < other.value<usize>(); i++) {
                        res.append(string());
                    }

                    return res;
                }
                default:
                    throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for *: '{}' and '{}'", type(), other.type()));
            }
        }
        default:
            throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for *: '{}' and '{}'", type(), other.type()));
    }
}

auto Value::operator%(const Value& other) const -> Value
{
    switch (typeInternal()) {
        case TypeInternal::Int: {
            switch (other.typeInternal()) {
                case TypeInternal::Int: {
                    return value<i64>() % other.value<i64>();
                }
                default:
                    throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for %: '{}' and '{}'", type(), other.type()));
            }
        }
        default:
            throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for %: '{}' and '{}'", type(), other.type()));
    }
}

auto Value::operator!() const noexcept -> Value
{
    return !toBool();
}

auto Value::operator~() const -> Value
{
    switch (typeInternal()) {
        case TypeInternal::Int:
            return ~value<i64>();
        default:
            throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand type for ~: '{}'", type()));
    }
}

auto Value::operator-() const -> Value
{
    switch (typeInternal()) {
        case TypeInternal::Int:
            return -value<i64>();
        case TypeInternal::Float:
            return -value<f64>();
        default:
            throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand type for -: '{}'", type()));
    }
}

auto Value::operator+() const -> Value
{
    switch (typeInternal()) {
        case TypeInternal::Int:
            return +value<i64>();
        case TypeInternal::Float:
            return +value<f64>();
        default:
            throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand type for +: '{}'", type()));
    }
}

auto Value::operator==(const Value& other) const noexcept -> bool
{
    switch (typeInternal()) {
        case TypeInternal::Bool: {
            switch (other.typeInternal()) {
                case TypeInternal::Bool: {
                    return value<bool>() == other.value<bool>();
                }
                default:
                    return false;
            }
        }
        case TypeInternal::Float: {
            switch (other.typeInternal()) {
                case TypeInternal::Float: {
                    return value<f64>() == other.value<f64>();
                }
                case TypeInternal::Int: {
                    return value<f64>() == static_cast<f64>(other.value<i64>());
                }
                default:
                    return false;
            }
        }
        case TypeInternal::Int: {
            switch (other.typeInternal()) {
                case TypeInternal::Float: {
                    return value<i64>() == static_cast<i64>(other.value<f64>());
                }
                case TypeInternal::Int: {
                    return value<i64>() == other.value<i64>();
                }
                default:
                    return false;
            }
        }
        case TypeInternal::Object: {
            switch (other.typeInternal()) {
                case TypeInternal::Object: {
                    return object() == other.object();
                }
                default:
                    return false;
            }
        }
        case TypeInternal::None: {
            return other.typeInternal() == TypeInternal::None;
        }
        case TypeInternal::String: {
            return other.typeInternal() == TypeInternal::String && string() == other.string();
        }
        default:
            return false;
    }
}

auto Value::operator!=(const Value& other) const noexcept -> bool
{
    return !(*this == other);
}

auto Value::operator<(const Value& other) const -> bool
{
    switch (typeInternal()) {
        case TypeInternal::Float: {
            switch (other.typeInternal()) {
                case TypeInternal::Float: {
                    return value<f64>() < other.value<f64>();
                }
                case TypeInternal::Int: {
                    return value<f64>() < static_cast<f64>(other.value<i64>());
                }
                default:
                    throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand type for <: '{}' and '{}'", type(), other.type()));
            }
        }
        case TypeInternal::Int: {
            switch (other.typeInternal()) {
                case TypeInternal::Float: {
                    return value<i64>() < static_cast<i64>(other.value<f64>());
                }
                case TypeInternal::Int: {
                    return value<i64>() < other.value<i64>();
                }
                default:
                    throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand type for <: '{}' and '{}'", type(), other.type()));
            }
        }
        default:
            throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand type for <: '{}' and '{}'", type(), other.type()));
    }
}

auto Value::operator<=(const Value& other) const -> bool
{
    switch (typeInternal()) {
        case TypeInternal::Float: {
            switch (other.typeInternal()) {
                case TypeInternal::Float: {
                    return value<f64>() <= other.value<f64>();
                }
                case TypeInternal::Int: {
                    return value<f64>() <= static_cast<f64>(other.value<i64>());
                }
                default:
                    throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand type for <=: '{}' and '{}'", type(), other.type()));
            }
        }
        case TypeInternal::Int: {
            switch (other.typeInternal()) {
                case TypeInternal::Float: {
                    return value<i64>() <= static_cast<i64>(other.value<f64>());
                }
                case TypeInternal::Int: {
                    return value<i64>() <= other.value<i64>();
                }
                default:
                    throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand type for <=: '{}' and '{}'", type(), other.type()));
            }
        }
        default:
            throw objects::PoiseException(objects::PoiseException::ExceptionType::InvalidOperand, fmt::format("Invalid operand type for <=: '{}' and '{}'", type(), other.type()));
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

auto Value::operator||(const Value& other) const noexcept -> bool
{
    return toBool() || other.toBool();
}

auto Value::operator&&(const Value& other) const noexcept -> bool
{
    return toBool() && other.toBool();
}

auto Value::data() const noexcept -> decltype(m_data)
{
    return m_data;
}

auto Value::makeNone() noexcept -> void
{
    m_data.none = std::nullptr_t{};
    m_type = TypeInternal::None;
}
}   // namespace poise::runtime

namespace fmt {
using namespace poise::runtime;
auto formatter<Value>::format(const Value& value, format_context& context) const -> decltype(context.out())
{
    return formatter<string_view>::format(value.toString(), context);
}
}
