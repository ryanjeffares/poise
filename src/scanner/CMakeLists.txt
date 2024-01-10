add_library(poise-scanner
    Scanner.cpp
    Token.cpp
    TokenType.cpp
    Scanner.hpp
    Token.hpp
    TokenType.hpp
)

target_compile_definitions(poise-scanner PRIVATE ${POISE_COMPILE_DEFINITIONS})
target_compile_options(poise-scanner PRIVATE ${POISE_COMPILE_OPTIONS})
target_include_directories(poise-scanner PRIVATE ${POISE_INCLUDE_DIRECTORIES})
target_link_libraries(poise-scanner PRIVATE fmt::fmt)
