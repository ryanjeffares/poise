#include "Compiler.hpp"
#include "Compiler_Macros.hpp"
#include "../objects/Exception.hpp"

#define TRY(expression)                                                                         \
    do {                                                                                        \
        try {                                                                                   \
            expression;                                                                         \
        } catch (const objects::Exception& exception) {                                         \
            errorAtPrevious(                                                                    \
                fmt::format("Error evaluating constant expression: {}", exception.message())    \
            );                                                                                  \
            return {};                                                                          \
        }                                                                                       \
    } while (false)

namespace poise::compiler {
auto Compiler::constantExpression() -> std::optional<runtime::Value>
{
    return constantLogicOr();
}

auto Compiler::constantLogicOr() -> std::optional<runtime::Value>
{
    if (auto lhs = constantLogicAnd()) {
        while (match(scanner::TokenType::Or)) {
            if (auto rhs = constantLogicAnd()) {
                TRY(lhs = *lhs || *rhs);
            } else {
                return {};
            }
        }

        return lhs;
    }

    return {};
}

auto Compiler::constantLogicAnd() -> std::optional<runtime::Value>
{
    if (auto lhs = constantBitwiseOr()) {
        while (match(scanner::TokenType::And)) {
            if (auto rhs = constantBitwiseOr()) {
                TRY(lhs = *lhs && *rhs);
            } else {
                return {};
            }
        }

        return lhs;
    }

    return {};
}

auto Compiler::constantBitwiseOr() -> std::optional<runtime::Value>
{
    if (auto lhs = constantBitwiseXor()) {
        while (match(scanner::TokenType::Pipe)) {
            if (auto rhs = constantBitwiseXor()) {
                TRY(lhs = *lhs | *rhs);
            } else {
                return {};
            }
        }

        return lhs;
    }

    return {};
}

auto Compiler::constantBitwiseXor() -> std::optional<runtime::Value>
{
    if (auto lhs = constantBitwiseAnd()) {
        while (match(scanner::TokenType::Caret)) {
            if (auto rhs = constantBitwiseAnd()) {
                TRY(lhs = *lhs ^ *rhs);
            } else {
                return {};
            }
        }

        return lhs;
    }

    return {};
}

auto Compiler::constantBitwiseAnd() -> std::optional<runtime::Value>
{
    if (auto lhs = constantEquality()) {
        while (match(scanner::TokenType::Ampersand)) {
            if (auto rhs = constantEquality()) {
                TRY(lhs = *lhs & *rhs);
            } else {
                return {};
            }
        }

        return lhs;
    }

    return {};
}

auto Compiler::constantEquality() -> std::optional<runtime::Value>
{
    if (auto lhs = constantComparison()) {
        if (match(scanner::TokenType::EqualEqual)) {
            if (auto rhs = constantComparison()) {
                TRY(lhs = *lhs == *rhs);
            } else {
                return {};
            }
        } else if (match(scanner::TokenType::NotEqual)) {
            if (auto rhs = constantComparison()) {
                TRY(lhs = *lhs != *rhs);
            } else {
                return {};
            }
        }

        return lhs;
    }

    return {};
}

auto Compiler::constantComparison() -> std::optional<runtime::Value>
{
    if (auto lhs = constantShift()) {
        if (match(scanner::TokenType::Less)) {
            if (auto rhs = constantShift()) {
                TRY(lhs = *lhs < *rhs);
            } else {
                return {};
            }
        } else if (match(scanner::TokenType::LessEqual)) {
            if (auto rhs = constantShift()) {
                TRY(lhs = *lhs <= *rhs);
            } else {
                return {};
            }
        } else if (match(scanner::TokenType::Greater)) {
            if (auto rhs = constantShift()) {
                TRY(lhs = *lhs > *rhs);
            } else {
                return {};
            }
        } else if (match(scanner::TokenType::GreaterEqual)) {
            if (auto rhs = constantShift()) {
                TRY(lhs = *lhs >= *rhs);
            } else {
                return {};
            }
        }

        return lhs;
    }

    return {};
}

auto Compiler::constantShift() -> std::optional<runtime::Value>
{
    if (auto lhs = constantTerm()) {
        while (true) {
            if (match(scanner::TokenType::ShiftLeft)) {
                if (auto rhs = constantTerm()) {
                    TRY(lhs = *lhs << *rhs);
                } else {
                    return {};
                }
            } else if (match(scanner::TokenType::ShiftRight)) {
                if (auto rhs = constantTerm()) {
                    TRY(lhs = *lhs >> *rhs);
                } else {
                    return {};
                }
            } else {
                break;
            }
        }

        return lhs;
    }

    return {};
}

auto Compiler::constantTerm() -> std::optional<runtime::Value>
{
    if (auto lhs = constantFactor()) {
        while (true) {
            if (match(scanner::TokenType::Plus)) {
                if (auto rhs = constantFactor()) {
                    TRY(lhs = *lhs + *rhs);
                } else {
                    return {};
                }
            } else if (match(scanner::TokenType::Minus)) {
                if (auto rhs = constantFactor()) {
                    TRY(lhs = *lhs - *rhs);
                } else {
                    return {};
                }
            } else {
                break;
            }
        }

        return lhs;
    }

    return {};
}

auto Compiler::constantFactor() -> std::optional<runtime::Value>
{
    if (auto lhs = constantUnary()) {
        while (true) {
            if (match(scanner::TokenType::Star)) {
                if (auto rhs = constantFactor()) {
                    TRY(lhs = *lhs * *rhs);
                } else {
                    return {};
                }
            } else if (match(scanner::TokenType::Slash)) {
                if (auto rhs = constantFactor()) {
                    TRY(lhs = *lhs / *rhs);
                } else {
                    return {};
                }
            } else if (match(scanner::TokenType::Modulus)) {
                if (auto rhs = constantFactor()) {
                    TRY(lhs = *lhs % *rhs);
                } else {
                    return {};
                }
            } else {
                break;
            }
        }

        return lhs;
    }

    return {};
}

auto Compiler::constantUnary() -> std::optional<runtime::Value>
{
    if (match(scanner::TokenType::Minus)) {
        if (auto rhs = constantUnary()) {
            TRY(return -*rhs);
        } else {
            return {};
        }
    } else if (match(scanner::TokenType::Tilde)) {
        if (auto rhs = constantUnary()) {
            TRY(return ~*rhs);
        } else {
            return {};
        }
    } else if (match(scanner::TokenType::Exclamation)) {
        if (auto rhs = constantUnary()) {
            TRY(return !*rhs);
        } else {
            return {};
        }
    } else if (match(scanner::TokenType::Plus)) {
        if (auto rhs = constantUnary()) {
            TRY(return +*rhs);
        } else {
            return {};
        }
    } else {
        return constantPrimary();
    }
}

auto Compiler::constantPrimary() -> std::optional<runtime::Value>
{
    if (match(scanner::TokenType::False)) {
        return false;
    } else if (match(scanner::TokenType::True)) {
        return true;
    } else if (match(scanner::TokenType::Float)) {
        return parseFloat();
    } else if (match(scanner::TokenType::Int)) {
        return parseInt();
    } else if (match(scanner::TokenType::None)) {
        return runtime::Value::none();
    } else if (match(scanner::TokenType::String)) {
        return parseString();
    } else if (match(scanner::TokenType::OpenParen)) {
        auto value = constantExpression();
        RETURN_VALUE_IF_NO_MATCH(scanner::TokenType::CloseParen, "Expected ')'", {});
        return value;
    } else if (match(scanner::TokenType::Identifier)) {
        return {};
    } else {
        errorAtCurrent("Invalid token in constant expression");
        return {};
    }
}
} // namespace poise::compiler

