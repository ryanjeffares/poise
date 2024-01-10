//
// Created by ryand on 03/12/2023.
//

#include "PoiseException.hpp"

namespace poise::objects {
PoiseException::PoiseException(std::string message)
    : m_exceptionType{ExceptionType::Exception}
    , m_message{std::move(message)}
{

}

PoiseException::PoiseException(PoiseException::ExceptionType exceptionType, std::string message)
    : m_exceptionType{exceptionType}
    , m_message{std::move(message)}
{

}

auto PoiseException::toString() const noexcept -> std::string
{
    return fmt::format("{}: {}", exceptionType(), message());
}

auto PoiseException::type() const noexcept -> runtime::types::Type
{
    return runtime::types::Type::Exception;
}

auto PoiseException::asException() noexcept -> PoiseException*
{
    return this;
}

auto PoiseException::what() const noexcept -> const char*
{
    return m_message.c_str();
}

auto PoiseException::exceptionType() const noexcept -> ExceptionType
{
    return m_exceptionType;
}

auto PoiseException::message() const noexcept -> std::string_view
{
    return m_message;
}
}   // namespace poise::objects

namespace fmt
{
using namespace poise::objects;

auto formatter<PoiseException::ExceptionType>::format(PoiseException::ExceptionType exceptionType, fmt::format_context& context) -> decltype(context.out())
{
    string_view res = "unknown";

    switch (exceptionType) {
        case PoiseException::ExceptionType::ArgumentOutOfRange:
            res = "ArgumentOutOfRangeException";
            break;
        case PoiseException::ExceptionType::AmbiguousCall:
            res = "AmbiguousCallException";
            break;
        case PoiseException::ExceptionType::Exception:
            res = "Exception";
            break;
        case PoiseException::ExceptionType::FunctionNotFound:
            res = "FunctionNotFoundException";
            break;
        case PoiseException::ExceptionType::IncorrectArgCount:
            res = "IncorrectArgCountException";
            break;
        case PoiseException::ExceptionType::IndexOutOfBounds:
            res = "IndexOutOfBoundsException";
            break;
        case PoiseException::ExceptionType::InvalidArgument:
            res = "InvalidArgumentException";
            break;
        case PoiseException::ExceptionType::InvalidCast:
            res = "InvalidCastException";
            break;
        case PoiseException::ExceptionType::InvalidIterator:
            res = "InvalidIteratorException";
            break;
        case PoiseException::ExceptionType::InvalidOperand:
            res = "InvalidOperandException";
            break;
        case PoiseException::ExceptionType::InvalidType:
            res = "InvalidTypeException";
            break;
        case PoiseException::ExceptionType::IteratorOutOfBounds:
            res = "IteratorOutOfBoundsException";
            break;
        case PoiseException::ExceptionType::KeyNotFound:
            res = "KeyNotFoundException";
            break;
    }

    return formatter<string_view>::format(res, context);
}
}   // namespace fmt
