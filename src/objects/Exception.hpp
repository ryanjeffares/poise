//
// Created by ryand on 03/12/2023.
//

#ifndef POISE_EXCEPTION_HPP
#define POISE_EXCEPTION_HPP

#include "Object.hpp"

#include <exception>

namespace poise::objects {
class Exception : public Object, public std::exception
{
public:
    enum class ExceptionType
    {
        AmbiguousCall,
        ArgumentOutOfRange,
        AssertionFailed,
        DivisionByZero,
        Exception,
        TypeNotExported,
        TypeNotFound,
        IncorrectArgCount,
        IndexOutOfBounds,
        InvalidArgument,
        InvalidCast,
        InvalidIterator,
        InvalidOperand,
        InvalidType,
        IteratorOutOfBounds,
        KeyNotFound,

        NumExceptionTypes,
    };

    explicit Exception(std::string message);
    explicit Exception(ExceptionType exceptionType);
    Exception(ExceptionType exceptionType, std::string message);

    ~Exception() override = default;

    [[nodiscard]] auto toString() const noexcept -> std::string override;
    [[nodiscard]] auto type() const noexcept -> runtime::types::Type override;
    auto findObjectMembers(std::unordered_set<Object*>& objects) const noexcept -> void override;
    auto removeObjectMembers() noexcept -> void override;
    [[nodiscard]] auto anyMemberMatchesRecursive(const Object* object) const noexcept -> bool override;

    [[nodiscard]] auto asException() noexcept -> Exception* override;

    [[nodiscard]] auto what() const noexcept -> const char* override;

    [[nodiscard]] auto exceptionType() const noexcept -> ExceptionType;
    [[nodiscard]] auto message() const noexcept -> std::string_view;

private:
    ExceptionType m_exceptionType;
    std::string m_message;
};  // class PoiseException
}   // namespace poise::objects

template<>
struct fmt::formatter<poise::objects::Exception::ExceptionType> : formatter<string_view>
{
    auto format(poise::objects::Exception::ExceptionType exceptionType, format_context& context) const -> decltype(context.out());
};   // namespace fmt

#endif  // POISE_EXCEPTION_HPP
