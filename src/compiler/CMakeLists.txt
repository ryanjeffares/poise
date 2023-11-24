add_library(poise_compiler Compiler.cpp)

target_compile_definitions(poise_compiler PRIVATE ${POISE_COMPILE_DEFINITIONS})
target_compile_options(poise_compiler PRIVATE ${POISE_COMPILE_OPTIONS})
target_include_directories(poise_compiler PRIVATE ${POISE_INCLUDE_DIRECTORIES})
target_link_libraries(poise_compiler PRIVATE fmt::fmt)