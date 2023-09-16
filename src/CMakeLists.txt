add_executable(poise
    main.cpp
    scanner/Scanner.cpp
    scanner/Token.cpp
    compiler/Compiler.cpp
    runtime/Value.cpp
    runtime/Vm.cpp
    objects/PoiseFunction.cpp
    objects/PoiseObject.cpp
)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(poise PRIVATE POISE_DEBUG)
endif()

if (MSVC)
    target_compile_definitions(poise PRIVATE POISE_MSVC)
    target_compile_options(poise PRIVATE /W4 /WX /external:templates- /external:W0 /std:c++latest)
else()
    target_compile_definitions(poise PRIVATE POISE_UNIX)
    target_compile_options(poise PRIVATE -Wall -Werror -Wpedantic -Wextra -Wconversion -std=gnu++2b)
endif()

target_link_libraries(poise PRIVATE fmt::fmt)
