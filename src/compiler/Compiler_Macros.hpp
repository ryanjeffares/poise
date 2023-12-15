//
// Created by Ryan Jeffares on 15/12/2023.
//

#ifndef POISE_COMPILER_MACROS_HPP
#define POISE_COMPILER_MACROS_HPP

#define RETURN_IF_NO_MATCH(tokenType, message)          \
    do {                                                \
        if (!match(tokenType)) {                        \
            errorAtCurrent(message);                    \
            return;                                     \
        }                                               \
    } while (false)

#define RETURN_VALUE_IF_NO_MATCH(tokenType, message, returnValue)   \
    do {                                                            \
        if (!match(tokenType)) {                                    \
            errorAtCurrent(message);                                \
            return returnValue;                                     \
        }                                                           \
    } while (false)

#define EXPECT_SEMICOLON() RETURN_IF_NO_MATCH(scanner::TokenType::Semicolon, "Expected ';'")
#define EXPECT_SEMICOLON_RETURN_VALUE(returnValue) RETURN_VALUE_IF_NO_MATCH(scanner::TokenType::Semicolon, "Expected ';'", returnValue)

#endif  // #ifndef POISE_COMPILER_MACROS_HPP
