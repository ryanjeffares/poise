#include "Value.hpp"

#include <fmt/core.h>

namespace poise::runtime
{
    Value::Value()
        : m_type{Type::Null}
    {
        m_data.null = std::nullptr_t{};
    }

    Value::Value(const Value& other)
        : m_type{other.m_type}
    {
        if (type() == Type::String) {
            m_data.string = new std::string{other.value<std::string>()};
        } else {
            m_data = other.m_data;
        }
    }

    Value::Value(Value&& other) noexcept
        : m_data{other.data()}
        , m_type{other.type()}
    {
        if (type() == Type::String) {
            other.nullify();
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
                m_data.string = new std::string{other.value<std::string>()};
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
                other.nullify();
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

    auto Value::toString() const -> std::string
    {
        switch (type()) {
            case Type::Null:
                return "null";
            case Type::Object:
                return object()->toString();
            case Type::String:
                return value<std::string>();
        }

        return "unknown";
    }

    auto Value::data() const -> decltype(m_data)
    {
        return m_data;
    }

    auto Value::nullify() -> void
    {
        m_data.null = std::nullptr_t{};
        m_type = Type::Null;
    }
}

namespace fmt
{
    using namespace poise::runtime;

    auto formatter<Value>::format(const Value& value, format_context& context) const -> decltype(context.out())
    {
        return formatter<string_view>::format(value.toString(), context);
    }
}
