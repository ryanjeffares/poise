//
// Created by ryand on 03/12/2023.
//

#include "Exception.hpp"

namespace poise::objects {
Exception::Exception(std::string message)
    : m_exceptionType{ExceptionType::Exception}
    , m_message{std::move(message)}
{

}

Exception::Exception(ExceptionType exceptionType)
    : m_exceptionType{exceptionType}
    , m_message{fmt::format("{}", exceptionType)}
{

}

Exception::Exception(ExceptionType exceptionType, std::string message)
    : m_exceptionType{exceptionType}
    , m_message{std::move(message)}
{

}

auto Exception::toString() const noexcept -> std::string
{
    return fmt::format("{}: {}", exceptionType(), message());
}

auto Exception::type() const noexcept -> runtime::types::Type
{
    return runtime::types::Type::Exception;
}

auto Exception::findObjectMembers([[maybe_unused]] std::unordered_set<Object*>& objects) const noexcept -> void
{

}

auto Exception::removeObjectMembers() noexcept -> void
{

}

auto Exception::anyMemberMatchesRecursive(const Object* object) const noexcept -> bool
{
    return object == this;
}

auto Exception::asException() noexcept -> Exception*
{
    return this;
}

auto Exception::what() const noexcept -> const char*
{
    return m_message.c_str();
}

auto Exception::exceptionType() const noexcept -> ExceptionType
{
    return m_exceptionType;
}

auto Exception::message() const noexcept -> std::string_view
{
    return m_message;
}
}   // namespace poise::objects

using namespace poise::objects;

auto fmt::formatter<Exception::ExceptionType>::format(Exception::ExceptionType exceptionType, format_context& context) const -> decltype(context.out())
{
    switch (exceptionType) {
        case Exception::ExceptionType::ArgumentOutOfRange:
            return formatter<string_view>::format("ArgumentOutOfRangeException", context);
        case Exception::ExceptionType::AmbiguousCall:
            return formatter<string_view>::format("AmbiguousCallException", context);
        case Exception::ExceptionType::AssertionFailed:
            return formatter<string_view>::format("AssertionFailedException", context);
        case Exception::ExceptionType::DivisionByZero:
            return formatter<string_view>::format("DivisionByZeroException", context);
        case Exception::ExceptionType::Exception:
            return formatter<string_view>::format("Exception", context);
        case Exception::ExceptionType::TypeNotExported:
            return formatter<string_view>::format("FunctionNotExportedException", context);
        case Exception::ExceptionType::TypeNotFound:
            return formatter<string_view>::format("FunctionNotFoundException", context);
        case Exception::ExceptionType::IncorrectArgCount:
            return formatter<string_view>::format("IncorrectArgCountException", context);
        case Exception::ExceptionType::IndexOutOfBounds:
            return formatter<string_view>::format("IndexOutOfBoundsException", context);
        case Exception::ExceptionType::InvalidArgument:
            return formatter<string_view>::format("InvalidArgumentException", context);
        case Exception::ExceptionType::InvalidCast:
            return formatter<string_view>::format("InvalidCastException", context);
        case Exception::ExceptionType::InvalidIterator:
            return formatter<string_view>::format("InvalidIteratorException", context);
        case Exception::ExceptionType::InvalidOperand:
            return formatter<string_view>::format("InvalidOperandException", context);
        case Exception::ExceptionType::InvalidType:
            return formatter<string_view>::format("InvalidTypeException", context);
        case Exception::ExceptionType::IteratorOutOfBounds:
            return formatter<string_view>::format("IteratorOutOfBoundsException", context);
        case Exception::ExceptionType::KeyNotFound:
            return formatter<string_view>::format("KeyNotFoundException", context);
        case Exception::ExceptionType::NumExceptionTypes:
            return formatter<string_view>::format("NumExceptionTypesException", context);
        default:
            POISE_UNREACHABLE();
    }
}
