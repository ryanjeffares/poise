//
// Created by ryand on 03/12/2023.
//

#ifndef POISE_EXCEPTION_HPP
#define POISE_EXCEPTION_HPP

#include "../Poise.hpp"
#include "PoiseObject.hpp"

#include <exception>

namespace poise::objects {

class PoiseException : public PoiseObject, public std::exception
{
public:
    enum class ExceptionType
    {
        Exception, FunctionNotFound, IncorrectArgCount, InvalidIterator, InvalidOperand, InvalidType,
    };

    explicit PoiseException(std::string message);
    PoiseException(ExceptionType exceptionType, std::string message);
    ~PoiseException() override = default;

    [[nodiscard]] auto toString() const noexcept -> std::string override;
    [[nodiscard]] auto typeValue() const noexcept -> const runtime::Value& override;

    [[nodiscard]] auto asException() noexcept -> PoiseException* override;

    [[nodiscard]] auto what() const noexcept -> const char* override;

    [[nodiscard]] auto exceptionType() const noexcept -> ExceptionType;
    [[nodiscard]] auto message() const noexcept -> std::string_view;

private:
    ExceptionType m_exceptionType;
    std::string m_message;
};  // class PoiseException

}   // namespace poise::objects

namespace fmt {
template<>
struct formatter<poise::objects::PoiseException::ExceptionType> : formatter<string_view>
{
    auto format(poise::objects::PoiseException::ExceptionType exceptionType, format_context& context) -> decltype(context.out());
};
}   // namespace fmt

#endif  // POISE_EXCEPTION_HPP
