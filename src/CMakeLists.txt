add_executable(poise
    main.cpp
    scanner/Scanner.cpp
    scanner/Token.cpp
    compiler/Compiler.cpp
    runtime/Value.cpp
    objects/PoiseFunction.cpp
    objects/PoiseObject.cpp
)

find_package(fmt REQUIRED)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(poise PRIVATE POISE_DEBUG)
endif()

target_compile_options(poise PRIVATE -Wall -Werror -Wpedantic -Wextra -Wconversion -std=gnu++2b)
target_link_libraries(poise PRIVATE fmt::fmt)
