//
// Created by ryand on 03/12/2023.
//

#include "PoiseException.hpp"

namespace poise::objects {
Exception::Exception(std::string message)
    : m_exceptionType{ExceptionType::Exception}
    , m_message{std::move(message)}
{

}

Exception::Exception(Exception::ExceptionType exceptionType, std::string message)
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

namespace fmt
{
using namespace poise::objects;

auto formatter<Exception::ExceptionType>::format(Exception::ExceptionType exceptionType, fmt::format_context& context) -> decltype(context.out())
{
    string_view res = "unknown";

    switch (exceptionType) {
        case Exception::ExceptionType::ArgumentOutOfRange:
            res = "ArgumentOutOfRangeException";
            break;
        case Exception::ExceptionType::AmbiguousCall:
            res = "AmbiguousCallException";
            break;
        case Exception::ExceptionType::Exception:
            res = "Exception";
            break;
        case Exception::ExceptionType::FunctionNotFound:
            res = "FunctionNotFoundException";
            break;
        case Exception::ExceptionType::IncorrectArgCount:
            res = "IncorrectArgCountException";
            break;
        case Exception::ExceptionType::IndexOutOfBounds:
            res = "IndexOutOfBoundsException";
            break;
        case Exception::ExceptionType::InvalidArgument:
            res = "InvalidArgumentException";
            break;
        case Exception::ExceptionType::InvalidCast:
            res = "InvalidCastException";
            break;
        case Exception::ExceptionType::InvalidIterator:
            res = "InvalidIteratorException";
            break;
        case Exception::ExceptionType::InvalidOperand:
            res = "InvalidOperandException";
            break;
        case Exception::ExceptionType::InvalidType:
            res = "InvalidTypeException";
            break;
        case Exception::ExceptionType::IteratorOutOfBounds:
            res = "IteratorOutOfBoundsException";
            break;
        case Exception::ExceptionType::KeyNotFound:
            res = "KeyNotFoundException";
            break;
    }

    return formatter<string_view>::format(res, context);
}
}   // namespace fmt
