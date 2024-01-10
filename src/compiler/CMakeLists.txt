add_library(poise-compiler
        Compiler.cpp
        Compiler_Helpers.cpp
        Compiler_Declarations.cpp
        Compiler_Statements.cpp
        Compiler_Expressions.cpp
)

target_compile_definitions(poise-compiler PRIVATE ${POISE_COMPILE_DEFINITIONS})
target_compile_options(poise-compiler PRIVATE ${POISE_COMPILE_OPTIONS})
target_include_directories(poise-compiler PRIVATE ${POISE_INCLUDE_DIRECTORIES})
target_link_libraries(poise-compiler PRIVATE fmt::fmt)
