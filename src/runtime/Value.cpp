#include "Value.hpp"
#include "../objects/Exception.hpp"
#include "../objects/iterables/hashables/Set.hpp"

#include <charconv>
#include <functional>
#include <version>

namespace poise::runtime {
using objects::Exception;

Value::Value()
    : m_type{TypeInternal::None}
{
    m_data.none = std::nullptr_t{};
}

Value::Value(const Value& other)
    : m_type{other.typeInternal()}
{
    if (typeInternal() == TypeInternal::String) {
#ifdef POISE_INTERN_STRINGS
        m_data.string = other.m_data.string;
#else
        m_data.string = new std::string{other.string()};
#endif
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
#ifndef POISE_INTERN_STRINGS
        if (typeInternal() == TypeInternal::String) {
            delete m_data.string;
        } else
#endif

        if (typeInternal() == TypeInternal::Object) {
            if (object()->decrementRefCount() == 0_uz) {
                memory::Gc::instance().stopTrackingObject(object());
                delete object();
            }
        }

        m_type = other.typeInternal();

        if (typeInternal() == TypeInternal::String) {
#ifdef POISE_INTERN_STRINGS
            m_data.string = other.m_data.string;
#else
            m_data.string = new std::string{other.string()};
#endif
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
#ifndef POISE_INTERN_STRINGS
        if (typeInternal() == TypeInternal::String) {
            delete m_data.string;
        } else
#endif

        if (typeInternal() == TypeInternal::Object) {
            if (object()->decrementRefCount() == 0_uz) {
                memory::Gc::instance().stopTrackingObject(object());
                delete object();
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
#ifndef POISE_INTERN_STRINGS
    if (typeInternal() == TypeInternal::String) {
        delete m_data.string;
    } else
#endif

    if (typeInternal() == TypeInternal::Object) {
        if (object()->decrementRefCount() == 0_uz) {
            memory::Gc::instance().stopTrackingObject(object());
            delete object();
        }
    }
}

auto Value::none() -> Value
{
    return std::nullptr_t{};
}

auto Value::string() const noexcept -> const std::string&
{
#ifdef POISE_INTERN_STRINGS
    return memory::findInternedString(m_data.string);
#else
    return *m_data.string;
#endif
}

auto Value::object() const noexcept -> objects::Object*
{
    return typeInternal() == TypeInternal::Object ? m_data.object : nullptr;
}

auto Value::type() const noexcept -> types::Type
{
    if (typeInternal() == TypeInternal::Object) {
        return object()->type();
    }

    return static_cast<types::Type>(typeInternal());
}

auto Value::hash() const noexcept -> usize
{
    switch (typeInternal()) {
        case TypeInternal::Bool:
            return std::hash<bool>{}(value<bool>());
        case TypeInternal::Float:
            return std::hash<f64>{}(value<f64>());
        case TypeInternal::Int:
            return std::hash<i64>{}(value<i64>());
        case TypeInternal::None:
            return std::hash<std::nullptr_t>{}(value<std::nullptr_t>());
        case TypeInternal::String:
            return std::hash<std::string>{}(string());
        case TypeInternal::Object:
            return std::hash<objects::Object*>{}(object());
        default:
            POISE_UNREACHABLE();
    }
}

auto Value::typeInternal() const noexcept -> TypeInternal
{
    return m_type;
}

auto Value::print(FILE* stream) const -> void
{
    fmt::print(stream, "{}", toString());
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
        case TypeInternal::String: {
#ifdef _LIBCPP_VERSION
            try {
                return std::stod(string());
            } catch (const std::invalid_argument&) {
                throw Exception(Exception::ExceptionType::InvalidCast, fmt::format("Cannot convert '{}' to Float", string()));
            } catch (const std::out_of_range&) {
                throw Exception(Exception::ExceptionType::InvalidCast, fmt::format("{} out of range for Float", string()));
            }
#else
            auto res{0.0};
            const auto [ptr, ec] = std::from_chars(string().data(), string().data() + string().length(), res);

            if (ec == std::errc::invalid_argument) {
                throw Exception(Exception::ExceptionType::InvalidCast, fmt::format("Cannot convert '{}' to Float", string()));
            }

            if (ec == std::errc::result_out_of_range) {
                throw Exception(Exception::ExceptionType::InvalidCast, fmt::format("{} out of range for Float", string()));
            }

            return res;
#endif
        }
        default:
            throw Exception(Exception::ExceptionType::InvalidType, fmt::format("Cannot convert {} to Float", type()));
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
        case TypeInternal::String: {
            auto res{0_i64};
            const auto [ptr, ec] = std::from_chars(string().data(), string().data() + string().length(), res);

            if (ec == std::errc::invalid_argument) {
                throw Exception(Exception::ExceptionType::InvalidCast, fmt::format("Cannot convert '{}' to Int", string()));
            }

            if (ec == std::errc::result_out_of_range) {
                throw Exception(Exception::ExceptionType::InvalidCast, fmt::format("{} out of range for Int", string()));
            }

            return res;
        }
        default:
            throw Exception(Exception::ExceptionType::InvalidType, fmt::format("Cannot convert {} to Int", type()));
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
                    throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for |: '{}' and '{}'", type(), other.type()));
            }
        }
        case TypeInternal::Object: {
            if (type() == types::Type::Set && other.type() == types::Type::Set) {
                return object()->asSet()->unionWith(*other.object()->asSet());
            }

            [[fallthrough]];
        }
        default:
            throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for |: '{}' and '{}'", type(), other.type()));
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
                    throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for ^: '{}' and '{}'", type(), other.type()));
            }
        }
        case TypeInternal::Object: {
            if (type() == types::Type::Set && other.type() == types::Type::Set) {
                return object()->asSet()->symmetricDifference(*other.object()->asSet());
            }

            [[fallthrough]];
        }
        default:
            throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for ^: '{}' and '{}'", type(), other.type()));
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
                    throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for &: '{}' and '{}'", type(), other.type()));
            }
        }
        case TypeInternal::Object: {
            if (type() == types::Type::Set && other.type() == types::Type::Set) {
                return object()->asSet()->intersection(*other.object()->asSet());
            }

            [[fallthrough]];
        }
        default:
            throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for &: '{}' and '{}'", type(), other.type()));
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
                    throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for <<: '{}' and '{}'", type(), other.type()));
            }
        }
        default:
            throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for <<: '{}' and '{}'", type(), other.type()));
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
                    throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for >>: '{}' and '{}'", type(), other.type()));
            }
        }
        default:
            throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for >>: '{}' and '{}'", type(), other.type()));
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
                    throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for +: '{}' and '{}'", type(), other.type()));
            }
        }
        case TypeInternal::Int: {
            switch (other.typeInternal()) {
                case TypeInternal::Float:
                    return static_cast<f64>(value<i64>()) + other.value<f64>();
                case TypeInternal::Int:
                    return value<i64>() + other.value<i64>();
                default:
                    throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for +: '{}' and '{}'", type(), other.type()));
            }
        }
        case TypeInternal::String:
            return string() + other.toString();
        default:
            throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for +: '{}' and '{}'", type(), other.type()));
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
                    throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for -: '{}' and '{}'", type(), other.type()));
            }
        }
        case TypeInternal::Int: {
            switch (other.typeInternal()) {
                case TypeInternal::Float:
                    return static_cast<f64>(value<i64>()) - other.value<f64>();
                case TypeInternal::Int:
                    return value<i64>() - other.value<i64>();
                default:
                    throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for -: '{}' and '{}'", type(), other.type()));
            }
        }
        case TypeInternal::Object: {
            if (type() == types::Type::Set && other.type() == types::Type::Set) {
                return object()->asSet()->difference(*other.object()->asSet());
            }

            [[fallthrough]];
        }
        default:
            throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for -: '{}' and '{}'", type(), other.type()));
    }
}

auto Value::operator/(const Value& other) const -> Value
{
    switch (typeInternal()) {
        case TypeInternal::Float: {
            switch (other.typeInternal()) {
                case TypeInternal::Float: {
                    if (other.value<f64>() == 0.0) {
                        throw Exception(Exception::ExceptionType::DivisionByZero);
                    }
                    return value<f64>() / other.value<f64>();
                }
                case TypeInternal::Int: {
                    if (other.value<i64>() == 0_i64) {
                        throw Exception(Exception::ExceptionType::DivisionByZero);
                    }
                    return value<f64>() / static_cast<f64>(other.value<i64>());
                }
                default:
                    throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for /: '{}' and '{}'", type(), other.type()));
            }
        }
        case TypeInternal::Int: {
            switch (other.typeInternal()) {
                case TypeInternal::Float: {
                    if (other.value<f64>() == 0.0) {
                        throw Exception(Exception::ExceptionType::DivisionByZero);
                    }
                    return static_cast<f64>(value<i64>()) / other.value<f64>();
                }
                case TypeInternal::Int: {
                    if (other.value<i64>() == 0_i64) {
                        throw Exception(Exception::ExceptionType::DivisionByZero);
                    }
                    return value<i64>() / other.value<i64>();
                }
                default:
                    throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for /: '{}' and '{}'", type(), other.type()));
            }
        }
        default:
            throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for /: '{}' and '{}'", type(), other.type()));
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
                    throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for *: '{}' and '{}'", type(), other.type()));
            }
        }
        case TypeInternal::Int: {
            switch (other.typeInternal()) {
                case TypeInternal::Float:
                    return static_cast<f64>(value<i64>()) * other.value<f64>();
                case TypeInternal::Int:
                    return value<i64>() * other.value<i64>();
                default:
                    throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for *: '{}' and '{}'", type(), other.type()));
            }
        }
        case TypeInternal::String: {
            switch (other.typeInternal()) {
                case TypeInternal::Int: {
                    if (other.value<i64>() < 0) {
                        throw Exception(Exception::ExceptionType::InvalidOperand, "Factor to repeat string cannot be type");
                    }

                    std::string res;
                    res.reserve(string().size() * other.value<usize>());
                    for (auto i = 0_uz; i < other.value<usize>(); i++) {
                        res.append(string());
                    }

                    return res;
                }
                default:
                    throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for *: '{}' and '{}'", type(), other.type()));
            }
        }
        default:
            throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for *: '{}' and '{}'", type(), other.type()));
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
                    throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for %: '{}' and '{}'", type(), other.type()));
            }
        }
        default:
            throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand types for %: '{}' and '{}'", type(), other.type()));
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
            throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand type for ~: '{}'", type()));
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
            throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand type for -: '{}'", type()));
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
            throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand type for +: '{}'", type()));
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
#ifdef POISE_INTERN_STRINGS
            return other.typeInternal() == TypeInternal::String && data().string == other.data().string;
#else
            return other.typeInternal() == TypeInternal::String && string() == other.string();
#endif
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
                    throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand type for <: '{}' and '{}'", type(), other.type()));
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
                    throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand type for <: '{}' and '{}'", type(), other.type()));
            }
        }
        default:
            throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand type for <: '{}' and '{}'", type(), other.type()));
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
                    throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand type for <=: '{}' and '{}'", type(), other.type()));
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
                    throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand type for <=: '{}' and '{}'", type(), other.type()));
            }
        }
        case TypeInternal::Object: {
            if (type() == types::Type::Set && other.type() == types::Type::Set) {
                return object()->asSet()->isSubset(*other.object()->asSet());
            }

            [[fallthrough]];
        }
        default:
            throw Exception(Exception::ExceptionType::InvalidOperand, fmt::format("Invalid operand type for <=: '{}' and '{}'", type(), other.type()));
    }
}

auto Value::operator>(const Value& other) const -> bool
{
    return !(*this <= other);
}

auto Value::operator>=(const Value& other) const -> bool
{
    if (type() == types::Type::Set && other.type() == types::Type::Set) {
        return object()->asSet()->isSuperset(*other.object()->asSet());
    }

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
