add_library(poise_scanner
    Scanner.cpp
    Token.cpp
    TokenType.cpp
    Scanner.hpp
    Token.hpp
    TokenType.hpp
)

target_compile_definitions(poise_scanner PRIVATE ${POISE_COMPILE_DEFINITIONS})
target_compile_options(poise_scanner PRIVATE ${POISE_COMPILE_OPTIONS})
target_include_directories(poise_scanner PRIVATE ${POISE_INCLUDE_DIRECTORIES})
target_link_libraries(poise_scanner PRIVATE fmt::fmt)
